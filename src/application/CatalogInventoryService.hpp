#pragma once

#include "application/InventoryService.hpp"
#include "application/ports/IPackageCatalog.hpp"

#include <expected>
#include <memory>
#include <stdexcept>

namespace fmm::application {

class CatalogInventoryService : public InventoryService {
public:
  explicit CatalogInventoryService(
      std::shared_ptr<fmm::application::ports::IPackageCatalog> catalog)
      : m_catalog(std::move(catalog)) {
    if (!m_catalog) {
      throw std::invalid_argument("IPackageCatalog cannot be null");
    }
  }

  [[nodiscard]] auto
  getInventory(const std::stop_token& stoken = {},
               const std::function<void(int, const std::string&)>& progress_callback = {}) const
      -> std::expected<std::vector<fmm::domain::InstalledPackage>, InventoryError> override {

    constexpr int start_progress = 10;
    constexpr int end_progress = 100;

    if (stoken.stop_requested()) {
      return std::unexpected(InventoryError::Cancelled);
    }

    if (progress_callback) {
      progress_callback(start_progress, "Reading catalog...");
    }

    auto catalog_result = m_catalog->list();
    if (stoken.stop_requested()) {
      return std::unexpected(InventoryError::Cancelled);
    }

    if (!catalog_result) {
      switch (catalog_result.error()) {
      case ports::PackageCatalogError::NotFound:
      case ports::PackageCatalogError::IoError:
        return std::unexpected(InventoryError::SourceUnavailable);
      case ports::PackageCatalogError::InvalidFormat:
      case ports::PackageCatalogError::UnsupportedSchemaVersion:
        return std::unexpected(InventoryError::CorruptMetadata);
      default:
        return std::unexpected(InventoryError::Internal);
      }
    }

    if (progress_callback) {
      progress_callback(end_progress, "Catalog loaded.");
    }

    return catalog_result.value();
  }

private:
  std::shared_ptr<fmm::application::ports::IPackageCatalog> m_catalog;
};

} // namespace fmm::application
