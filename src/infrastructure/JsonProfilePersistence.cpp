#include "infrastructure/JsonProfilePersistence.hpp"

#include <QByteArray>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace fmm::infrastructure {

JsonProfilePersistence::JsonProfilePersistence(std::filesystem::path storage_directory)
    : m_storage_directory(std::move(storage_directory)) {}

auto JsonProfilePersistence::load(const domain::ProfileIdentity& identity)
    -> std::expected<domain::ProfileMetadata, domain::ProfilePersistenceError> {
  auto file_path = m_storage_directory / std::string(identity.name()) / "metadata.json";

  QFile file(QString::fromStdString(file_path.string()));
  if (!file.exists()) {
    return std::unexpected(domain::ProfilePersistenceError::FileNotFound);
  }

  if (!file.open(QIODevice::ReadOnly)) {
    return std::unexpected(domain::ProfilePersistenceError::AccessDenied);
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonParseError parse_error;
  QJsonDocument doc = QJsonDocument::fromJson(data, &parse_error);

  if (parse_error.error != QJsonParseError::NoError || !doc.isObject()) {
    return std::unexpected(domain::ProfilePersistenceError::InvalidFormat);
  }

  QJsonObject obj = doc.object();

  if (!obj.contains("schema_version") || !obj["schema_version"].isDouble()) {
    return std::unexpected(domain::ProfilePersistenceError::InvalidFormat);
  }

  int version = obj["schema_version"].toInt();
  if (version > CURRENT_SCHEMA_VERSION || version <= 0) {
    return std::unexpected(domain::ProfilePersistenceError::UnsupportedSchemaVersion);
  }

  if (!obj.contains("description") || !obj["description"].isString()) {
    return std::unexpected(domain::ProfilePersistenceError::InvalidFormat);
  }

  domain::ProfileMetadata metadata{.identity = identity,
                                   .schema_version = version,
                                   .description = obj["description"].toString().toStdString()};

  return metadata;
}

auto JsonProfilePersistence::save(const domain::ProfileMetadata& metadata)
    -> std::expected<void, domain::ProfilePersistenceError> {
  auto profile_dir = m_storage_directory / std::string(metadata.identity.name());

  std::error_code error_code;
  std::filesystem::create_directories(profile_dir, error_code);
  if (error_code) {
    return std::unexpected(domain::ProfilePersistenceError::WriteFailed);
  }

  auto file_path = profile_dir / "metadata.json";
  QFile file(QString::fromStdString(file_path.string()));

  if (!file.open(QIODevice::WriteOnly)) {
    return std::unexpected(domain::ProfilePersistenceError::AccessDenied);
  }

  QJsonObject obj;
  obj["schema_version"] = metadata.schema_version;
  obj["description"] = QString::fromStdString(metadata.description);

  QJsonDocument doc(obj);
  if (file.write(doc.toJson()) == -1) {
    return std::unexpected(domain::ProfilePersistenceError::WriteFailed);
  }

  return {};
}

} // namespace fmm::infrastructure
