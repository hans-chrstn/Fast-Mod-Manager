#include "infrastructure/JsonPackageCatalog.hpp"

#include <QFile>
#include <QTemporaryDir>
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <string_view>

using namespace fmm::domain;
using namespace fmm::application::ports;
using namespace fmm::infrastructure;

namespace {
class CatalogFixture {
public:
  CatalogFixture() { REQUIRE(m_directory.isValid()); }

  [[nodiscard]] auto catalogPath() const -> std::filesystem::path {
    const auto encoded_path = QFile::encodeName(m_directory.filePath("catalog.json"));
    return {std::string(encoded_path.constData(), static_cast<std::size_t>(encoded_path.size()))};
  }

  [[nodiscard]] auto directoryPath() const -> std::filesystem::path {
    const auto encoded_path = QFile::encodeName(m_directory.path());
    return {std::string(encoded_path.constData(), static_cast<std::size_t>(encoded_path.size()))};
  }

  void writeCatalog(std::string_view contents) const {
    QFile file(QString::fromStdString(catalogPath().string()));
    REQUIRE(file.open(QIODevice::WriteOnly));
    REQUIRE(file.write(contents.data(), static_cast<qint64>(contents.size())) ==
            static_cast<qint64>(contents.size()));
  }

private:
  QTemporaryDir m_directory;
};

auto makePackage(std::string package_id = "test_pkg_1",
                 std::filesystem::path staging_path = "/managed/test_pkg_1") -> InstalledPackage {
  return InstalledPackage(PackageId{std::move(package_id)},
                          PackageMetadata{.name = "Test Mod", .version = "1.0", .source = "Nexus"},
                          PackageLocation{.staging_path = std::move(staging_path),
                                          .object_store_ref = std::string("content-reference")});
}
} // namespace

TEST_CASE("JsonPackageCatalog treats a missing file as empty", "[unit][catalog]") {
  CatalogFixture fixture;
  JsonPackageCatalog catalog(fixture.catalogPath());

  const auto list_result = catalog.list();
  REQUIRE(list_result.has_value());
  REQUIRE(list_result->empty());

  const auto contains_result = catalog.contains(PackageId{"test_pkg"});
  REQUIRE(contains_result.has_value());
  REQUIRE_FALSE(*contains_result);
}

TEST_CASE("JsonPackageCatalog persists package metadata and location", "[unit][catalog]") {
  CatalogFixture fixture;
  JsonPackageCatalog catalog(fixture.catalogPath());
  const auto package = makePackage();

  REQUIRE(catalog.add(package).has_value());

  const auto contains_result = catalog.contains(package.id());
  REQUIRE(contains_result.has_value());
  REQUIRE(*contains_result);

  const auto get_result = catalog.get(package.id());
  REQUIRE(get_result.has_value());
  REQUIRE(*get_result == package);

  JsonPackageCatalog reloaded_catalog(fixture.catalogPath());
  const auto list_result = reloaded_catalog.list();
  REQUIRE(list_result.has_value());
  REQUIRE(*list_result == std::vector<InstalledPackage>{package});
}

TEST_CASE("JsonPackageCatalog preserves arbitrary POSIX path bytes", "[unit][catalog]") {
  CatalogFixture fixture;
  JsonPackageCatalog catalog(fixture.catalogPath());
  std::string native_path = "/managed/package_";
  native_path.push_back(static_cast<char>(0xFF));
  const auto package = makePackage("byte_path", std::filesystem::path(native_path));

  REQUIRE(catalog.add(package).has_value());
  const auto get_result = catalog.get(package.id());
  REQUIRE(get_result.has_value());
  REQUIRE(get_result->location().staging_path == package.location().staging_path);
}

TEST_CASE("JsonPackageCatalog rejects duplicate package IDs", "[unit][catalog]") {
  CatalogFixture fixture;
  JsonPackageCatalog catalog(fixture.catalogPath());
  const auto package = makePackage();

  REQUIRE(catalog.add(package).has_value());
  const auto duplicate_result = catalog.add(package);
  REQUIRE_FALSE(duplicate_result.has_value());
  REQUIRE(duplicate_result.error() == PackageCatalogError::AlreadyExists);
}

TEST_CASE("JsonPackageCatalog removes packages without deleting storage", "[unit][catalog]") {
  CatalogFixture fixture;
  JsonPackageCatalog catalog(fixture.catalogPath());
  const auto package = makePackage();

  REQUIRE(catalog.add(package).has_value());
  REQUIRE(catalog.remove(package.id()).has_value());

  const auto contains_result = catalog.contains(package.id());
  REQUIRE(contains_result.has_value());
  REQUIRE_FALSE(*contains_result);
  const auto list_result = catalog.list();
  REQUIRE(list_result.has_value());
  REQUIRE(list_result->empty());
}

TEST_CASE("JsonPackageCatalog reports a missing package", "[unit][catalog]") {
  CatalogFixture fixture;
  JsonPackageCatalog catalog(fixture.catalogPath());

  const auto get_result = catalog.get(PackageId{"does_not_exist"});
  REQUIRE_FALSE(get_result.has_value());
  REQUIRE(get_result.error() == PackageCatalogError::NotFound);

  const auto remove_result = catalog.remove(PackageId{"does_not_exist"});
  REQUIRE_FALSE(remove_result.has_value());
  REQUIRE(remove_result.error() == PackageCatalogError::NotFound);
}

TEST_CASE("JsonPackageCatalog validates persisted schema and package fields", "[unit][catalog]") {
  CatalogFixture fixture;
  JsonPackageCatalog catalog(fixture.catalogPath());

  SECTION("Malformed JSON") {
    fixture.writeCatalog("{not-json");
    const auto result = catalog.list();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == PackageCatalogError::InvalidFormat);
  }

  SECTION("Unsupported schema") {
    fixture.writeCatalog(R"({"schema_version":2,"packages":[]})");
    const auto result = catalog.list();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == PackageCatalogError::UnsupportedSchemaVersion);
  }

  SECTION("Non-integral schema") {
    fixture.writeCatalog(R"({"schema_version":1.5,"packages":[]})");
    const auto result = catalog.list();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == PackageCatalogError::InvalidFormat);
  }

  SECTION("Missing metadata field") {
    fixture.writeCatalog(
        R"({"schema_version":1,"packages":[{"id":"pkg","metadata":{"name":"Mod","version":"1"},"location":{"object_store_ref":"object"}}]})");
    const auto result = catalog.list();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == PackageCatalogError::InvalidFormat);
  }

  SECTION("Duplicate persisted IDs") {
    fixture.writeCatalog(
        R"({"schema_version":1,"packages":[{"id":"pkg","metadata":{"name":"One","version":"1","source":"test"},"location":{"object_store_ref":"one"}},{"id":"pkg","metadata":{"name":"Two","version":"1","source":"test"},"location":{"object_store_ref":"two"}}]})");
    const auto result = catalog.list();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == PackageCatalogError::InvalidFormat);
  }
}

TEST_CASE("JsonPackageCatalog does not hide corruption in contains", "[unit][catalog]") {
  CatalogFixture fixture;
  fixture.writeCatalog("{not-json");
  JsonPackageCatalog catalog(fixture.catalogPath());

  const auto result = catalog.contains(PackageId{"pkg"});
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error() == PackageCatalogError::InvalidFormat);
}

TEST_CASE("JsonPackageCatalog rejects invalid in-memory packages", "[unit][catalog]") {
  CatalogFixture fixture;
  JsonPackageCatalog catalog(fixture.catalogPath());

  const InstalledPackage empty_id(
      PackageId{""}, PackageMetadata{.name = "Mod", .version = "1", .source = "test"},
      PackageLocation{.staging_path = std::filesystem::path("/managed/mod")});
  const auto empty_id_result = catalog.add(empty_id);
  REQUIRE_FALSE(empty_id_result.has_value());
  REQUIRE(empty_id_result.error() == PackageCatalogError::InvalidFormat);

  const InstalledPackage missing_location(
      PackageId{"pkg"}, PackageMetadata{.name = "Mod", .version = "1", .source = "test"},
      PackageLocation{});
  const auto missing_location_result = catalog.add(missing_location);
  REQUIRE_FALSE(missing_location_result.has_value());
  REQUIRE(missing_location_result.error() == PackageCatalogError::InvalidFormat);
}

TEST_CASE("JsonPackageCatalog preserves the old catalog when atomic publish fails",
          "[unit][catalog]") {
  CatalogFixture fixture;
  JsonPackageCatalog catalog(fixture.catalogPath());
  const auto first_package = makePackage("first", "/managed/first");
  const auto second_package = makePackage("second", "/managed/second");
  REQUIRE(catalog.add(first_package).has_value());

  std::error_code permission_error;
  std::filesystem::permissions(fixture.directoryPath(),
                               std::filesystem::perms::owner_read |
                                   std::filesystem::perms::owner_exec,
                               std::filesystem::perm_options::replace, permission_error);
  REQUIRE_FALSE(permission_error);
  const auto add_result = catalog.add(second_package);
  std::filesystem::permissions(fixture.directoryPath(), std::filesystem::perms::owner_all,
                               std::filesystem::perm_options::replace, permission_error);
  REQUIRE_FALSE(permission_error);

  REQUIRE_FALSE(add_result.has_value());
  REQUIRE(add_result.error() == PackageCatalogError::IoError);
  const auto list_result = catalog.list();
  REQUIRE(list_result.has_value());
  REQUIRE(*list_result == std::vector<InstalledPackage>{first_package});
}
