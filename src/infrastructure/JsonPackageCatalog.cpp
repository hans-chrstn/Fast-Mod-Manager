#include "infrastructure/JsonPackageCatalog.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <algorithm>
#include <cmath>
#include <string_view>

using namespace fmm::domain;
using namespace fmm::application::ports;

namespace fmm::infrastructure {

namespace {
constexpr int SCHEMA_VERSION = 1;
constexpr unsigned char LOW_NIBBLE_MASK = 0x0FU;
constexpr unsigned char HEX_ALPHA_OFFSET = 10U;

auto encodePath(const std::filesystem::path& path) -> QString {
  constexpr std::string_view hex_digits = "0123456789abcdef";
  const auto& native_path = path.native();
  std::string encoded;
  encoded.reserve(native_path.size() * 2U);

  for (const char character : native_path) {
    const auto byte = static_cast<unsigned char>(character);
    encoded.push_back(hex_digits[byte >> 4U]);
    encoded.push_back(hex_digits[byte & LOW_NIBBLE_MASK]);
  }

  return QString::fromLatin1(encoded);
}

auto decodeHexDigit(char digit) -> std::optional<unsigned char> {
  if (digit >= '0' && digit <= '9') {
    return static_cast<unsigned char>(digit - '0');
  }
  if (digit >= 'a' && digit <= 'f') {
    return static_cast<unsigned char>(digit - 'a' + HEX_ALPHA_OFFSET);
  }
  if (digit >= 'A' && digit <= 'F') {
    return static_cast<unsigned char>(digit - 'A' + HEX_ALPHA_OFFSET);
  }
  return std::nullopt;
}

auto decodePath(const QString& encoded_path)
    -> std::expected<std::filesystem::path, PackageCatalogError> {
  const auto encoded = encoded_path.toLatin1().toStdString();
  if (encoded.empty() || encoded.size() % 2U != 0U) {
    return std::unexpected(PackageCatalogError::InvalidFormat);
  }

  std::string native_path;
  native_path.reserve(encoded.size() / 2U);
  for (std::size_t index = 0; index < encoded.size(); index += 2U) {
    const auto high = decodeHexDigit(encoded[index]);
    const auto low = decodeHexDigit(encoded[index + 1U]);
    if (!high || !low) {
      return std::unexpected(PackageCatalogError::InvalidFormat);
    }
    native_path.push_back(static_cast<char>((*high << 4U) | *low));
  }

  return std::filesystem::path(std::move(native_path));
}

auto validatePackage(const InstalledPackage& package) -> bool {
  const auto& location = package.location();
  return !package.id().value().empty() && !package.metadata().name.empty() &&
         (location.staging_path.has_value() || location.object_store_ref.has_value()) &&
         (!location.staging_path || !location.staging_path->empty()) &&
         (!location.object_store_ref || !location.object_store_ref->empty());
}

auto packageToJson(const InstalledPackage& package) -> QJsonObject {
  QJsonObject object;
  object["id"] = QString::fromStdString(package.id().value());

  QJsonObject metadata;
  metadata["name"] = QString::fromStdString(package.metadata().name);
  metadata["version"] = QString::fromStdString(package.metadata().version);
  metadata["source"] = QString::fromStdString(package.metadata().source);
  object["metadata"] = metadata;

  QJsonObject location;
  if (package.location().staging_path) {
    location["staging_path_hex"] = encodePath(*package.location().staging_path);
  }
  if (package.location().object_store_ref) {
    location["object_store_ref"] = QString::fromStdString(*package.location().object_store_ref);
  }
  object["location"] = location;

  return object;
}

auto jsonToPackage(const QJsonObject& object)
    -> std::expected<InstalledPackage, PackageCatalogError> {
  if (!object.contains("id") || !object["id"].isString() || object["id"].toString().isEmpty() ||
      !object.contains("metadata") || !object["metadata"].isObject() ||
      !object.contains("location") || !object["location"].isObject()) {
    return std::unexpected(PackageCatalogError::InvalidFormat);
  }

  const auto metadata_object = object["metadata"].toObject();
  if (!metadata_object.contains("name") || !metadata_object["name"].isString() ||
      metadata_object["name"].toString().isEmpty() || !metadata_object.contains("version") ||
      !metadata_object["version"].isString() || !metadata_object.contains("source") ||
      !metadata_object["source"].isString()) {
    return std::unexpected(PackageCatalogError::InvalidFormat);
  }

  const auto location_object = object["location"].toObject();
  const bool has_staging_path = location_object.contains("staging_path_hex");
  const bool has_object_store_ref = location_object.contains("object_store_ref");
  if ((!has_staging_path && !has_object_store_ref) ||
      (has_staging_path && !location_object["staging_path_hex"].isString()) ||
      (has_object_store_ref && (!location_object["object_store_ref"].isString() ||
                                location_object["object_store_ref"].toString().isEmpty()))) {
    return std::unexpected(PackageCatalogError::InvalidFormat);
  }

  PackageLocation location;
  if (has_staging_path) {
    auto decoded_path = decodePath(location_object["staging_path_hex"].toString());
    if (!decoded_path) {
      return std::unexpected(decoded_path.error());
    }
    location.staging_path = std::move(*decoded_path);
  }
  if (has_object_store_ref) {
    location.object_store_ref = location_object["object_store_ref"].toString().toStdString();
  }

  InstalledPackage package(
      PackageId{object["id"].toString().toStdString()},
      PackageMetadata{.name = metadata_object["name"].toString().toStdString(),
                      .version = metadata_object["version"].toString().toStdString(),
                      .source = metadata_object["source"].toString().toStdString()},
      std::move(location));
  if (!validatePackage(package)) {
    return std::unexpected(PackageCatalogError::InvalidFormat);
  }
  return package;
}
} // namespace

JsonPackageCatalog::JsonPackageCatalog(std::filesystem::path catalog_file)
    : m_catalog_file(std::move(catalog_file)) {}

auto JsonPackageCatalog::load() const
    -> std::expected<std::vector<InstalledPackage>, PackageCatalogError> {
  std::error_code filesystem_error;
  const bool catalog_exists = std::filesystem::exists(m_catalog_file, filesystem_error);
  if (filesystem_error) {
    return std::unexpected(PackageCatalogError::IoError);
  }
  if (!catalog_exists) {
    return std::vector<InstalledPackage>{};
  }

  QFile file(QString::fromStdString(m_catalog_file.string()));
  if (!file.open(QIODevice::ReadOnly)) {
    return std::unexpected(PackageCatalogError::IoError);
  }

  QJsonParseError parse_error{};
  const auto document = QJsonDocument::fromJson(file.readAll(), &parse_error);
  if (file.error() != QFileDevice::NoError) {
    return std::unexpected(PackageCatalogError::IoError);
  }
  if (parse_error.error != QJsonParseError::NoError || !document.isObject()) {
    return std::unexpected(PackageCatalogError::InvalidFormat);
  }

  const auto root = document.object();
  if (!root.contains("schema_version") || !root["schema_version"].isDouble()) {
    return std::unexpected(PackageCatalogError::InvalidFormat);
  }
  const double schema_value = root["schema_version"].toDouble();
  if (!std::isfinite(schema_value) || std::trunc(schema_value) != schema_value ||
      schema_value <= 0.0) {
    return std::unexpected(PackageCatalogError::InvalidFormat);
  }
  if (schema_value != static_cast<double>(SCHEMA_VERSION)) {
    return std::unexpected(PackageCatalogError::UnsupportedSchemaVersion);
  }

  if (!root.contains("packages") || !root["packages"].isArray()) {
    return std::unexpected(PackageCatalogError::InvalidFormat);
  }

  std::vector<InstalledPackage> packages;
  const auto package_array = root["packages"].toArray();
  packages.reserve(static_cast<std::size_t>(package_array.size()));
  for (const auto& item : package_array) {
    if (!item.isObject()) {
      return std::unexpected(PackageCatalogError::InvalidFormat);
    }
    auto package_result = jsonToPackage(item.toObject());
    if (!package_result ||
        std::ranges::any_of(packages, [&](const InstalledPackage& existing_package) -> bool {
          return existing_package.id() == package_result->id();
        })) {
      return std::unexpected(PackageCatalogError::InvalidFormat);
    }
    packages.push_back(std::move(*package_result));
  }

  return packages;
}

auto JsonPackageCatalog::save(const std::vector<InstalledPackage>& packages) const
    -> std::expected<void, PackageCatalogError> {
  QJsonArray package_array;
  for (const auto& package : packages) {
    if (!validatePackage(package)) {
      return std::unexpected(PackageCatalogError::InvalidFormat);
    }
    package_array.append(packageToJson(package));
  }

  QJsonObject root;
  root["schema_version"] = SCHEMA_VERSION;
  root["packages"] = package_array;

  std::error_code filesystem_error;
  const auto parent_directory = m_catalog_file.parent_path();
  if (!parent_directory.empty()) {
    std::filesystem::create_directories(parent_directory, filesystem_error);
    if (filesystem_error) {
      return std::unexpected(PackageCatalogError::IoError);
    }
  }

  const QJsonDocument document(root);
  const QByteArray serialized = document.toJson();
  QSaveFile file(QString::fromStdString(m_catalog_file.string()));
  file.setDirectWriteFallback(false);
  if (!file.open(QIODevice::WriteOnly) || file.write(serialized) != serialized.size() ||
      !file.commit()) {
    file.cancelWriting();
    return std::unexpected(PackageCatalogError::IoError);
  }

  return {};
}

auto JsonPackageCatalog::get(const PackageId& package_id) const
    -> std::expected<InstalledPackage, PackageCatalogError> {
  std::scoped_lock lock(m_mutex);
  auto packages_result = load();
  if (!packages_result) {
    return std::unexpected(packages_result.error());
  }

  const auto package_position =
      std::ranges::find_if(*packages_result, [&](const InstalledPackage& package) -> bool {
        return package.id() == package_id;
      });
  if (package_position == packages_result->end()) {
    return std::unexpected(PackageCatalogError::NotFound);
  }
  return *package_position;
}

auto JsonPackageCatalog::list() const
    -> std::expected<std::vector<InstalledPackage>, PackageCatalogError> {
  std::scoped_lock lock(m_mutex);
  return load();
}

auto JsonPackageCatalog::add(const InstalledPackage& package)
    -> std::expected<void, PackageCatalogError> {
  if (!validatePackage(package)) {
    return std::unexpected(PackageCatalogError::InvalidFormat);
  }

  std::scoped_lock lock(m_mutex);
  auto packages_result = load();
  if (!packages_result) {
    return std::unexpected(packages_result.error());
  }

  if (std::ranges::any_of(*packages_result, [&](const InstalledPackage& existing_package) -> bool {
        return existing_package.id() == package.id();
      })) {
    return std::unexpected(PackageCatalogError::AlreadyExists);
  }

  packages_result->push_back(package);
  return save(*packages_result);
}

auto JsonPackageCatalog::remove(const PackageId& package_id)
    -> std::expected<void, PackageCatalogError> {
  std::scoped_lock lock(m_mutex);
  auto packages_result = load();
  if (!packages_result) {
    return std::unexpected(packages_result.error());
  }

  auto& packages = *packages_result;
  const auto removed =
      std::ranges::remove_if(packages, [&](const InstalledPackage& package) -> bool {
        return package.id() == package_id;
      });

  if (removed.begin() == packages.end()) {
    return std::unexpected(PackageCatalogError::NotFound);
  }

  packages.erase(removed.begin(), packages.end());
  return save(packages);
}

auto JsonPackageCatalog::contains(const PackageId& package_id) const
    -> std::expected<bool, PackageCatalogError> {
  std::scoped_lock lock(m_mutex);
  auto packages_result = load();
  if (!packages_result) {
    return std::unexpected(packages_result.error());
  }

  return std::ranges::any_of(*packages_result, [&](const InstalledPackage& package) -> bool {
    return package.id() == package_id;
  });
}

} // namespace fmm::infrastructure
