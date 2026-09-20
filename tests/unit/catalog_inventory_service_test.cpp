#include "application/CatalogInventoryService.hpp"

#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <utility>
#include <vector>

using namespace fmm::application;
using namespace fmm::application::ports;
using namespace fmm::domain;

namespace {
class TestPackageCatalog final : public IPackageCatalog {
public:
  [[nodiscard]] auto get(const PackageId& package_id) const
      -> std::expected<InstalledPackage, PackageCatalogError> override {
    static_cast<void>(package_id);
    return std::unexpected(PackageCatalogError::NotFound);
  }

  [[nodiscard]] auto list() const
      -> std::expected<std::vector<InstalledPackage>, PackageCatalogError> override {
    ++list_calls;
    if (list_error) {
      return std::unexpected(*list_error);
    }
    return packages;
  }

  [[nodiscard]] auto add(const InstalledPackage& package)
      -> std::expected<void, PackageCatalogError> override {
    static_cast<void>(package);
    return {};
  }

  [[nodiscard]] auto remove(const PackageId& package_id)
      -> std::expected<void, PackageCatalogError> override {
    static_cast<void>(package_id);
    return std::unexpected(PackageCatalogError::NotFound);
  }

  [[nodiscard]] auto contains(const PackageId& package_id) const
      -> std::expected<bool, PackageCatalogError> override {
    static_cast<void>(package_id);
    return false;
  }

  std::vector<InstalledPackage> packages;
  std::optional<PackageCatalogError> list_error;
  mutable int list_calls{};
};

auto makeInventoryPackage() -> InstalledPackage {
  return InstalledPackage(
      PackageId{"inventory-package"},
      PackageMetadata{.name = "Inventory Package", .version = "1", .source = "test"},
      PackageLocation{.staging_path = std::filesystem::path("/managed/inventory-package")});
}
} // namespace

TEST_CASE("CatalogInventoryService rejects a null catalog", "[unit][inventory][catalog]") {
  REQUIRE_THROWS_AS(CatalogInventoryService(nullptr), std::invalid_argument);
}

TEST_CASE("CatalogInventoryService returns catalog packages and progress",
          "[unit][inventory][catalog]") {
  auto catalog = std::make_shared<TestPackageCatalog>();
  catalog->packages.push_back(makeInventoryPackage());
  CatalogInventoryService service(catalog);
  std::vector<std::pair<int, std::string>> progress;

  const auto result = service.getInventory({}, [&](int percentage, const std::string& message) {
    progress.emplace_back(percentage, message);
  });

  REQUIRE(result.has_value());
  REQUIRE(*result == catalog->packages);
  REQUIRE(progress == std::vector<std::pair<int, std::string>>{{10, "Reading catalog..."},
                                                               {100, "Catalog loaded."}});
}

TEST_CASE("CatalogInventoryService honors cancellation before catalog access",
          "[unit][inventory][catalog]") {
  auto catalog = std::make_shared<TestPackageCatalog>();
  CatalogInventoryService service(catalog);
  std::stop_source stop_source;
  stop_source.request_stop();

  const auto result = service.getInventory(stop_source.get_token());

  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error() == InventoryError::Cancelled);
  REQUIRE(catalog->list_calls == 0);
}

TEST_CASE("CatalogInventoryService maps persistence errors", "[unit][inventory][catalog]") {
  auto catalog = std::make_shared<TestPackageCatalog>();
  CatalogInventoryService service(catalog);

  SECTION("I/O failure is source unavailable") {
    catalog->list_error = PackageCatalogError::IoError;
    const auto result = service.getInventory();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == InventoryError::SourceUnavailable);
  }

  SECTION("Invalid catalog is corrupt metadata") {
    catalog->list_error = PackageCatalogError::InvalidFormat;
    const auto result = service.getInventory();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == InventoryError::CorruptMetadata);
  }

  SECTION("Unsupported schema is corrupt metadata") {
    catalog->list_error = PackageCatalogError::UnsupportedSchemaVersion;
    const auto result = service.getInventory();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == InventoryError::CorruptMetadata);
  }
}
