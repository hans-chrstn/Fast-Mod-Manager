#pragma once

#include "application/ports/IPackageCatalog.hpp"

#include <filesystem>
#include <mutex>

namespace fmm::infrastructure {

class JsonPackageCatalog : public application::ports::IPackageCatalog {
public:
  explicit JsonPackageCatalog(std::filesystem::path catalog_file);
  ~JsonPackageCatalog() override = default;

  [[nodiscard]] auto get(const domain::PackageId& package_id) const
      -> std::expected<domain::InstalledPackage, application::ports::PackageCatalogError> override;

  [[nodiscard]] auto list() const
      -> std::expected<std::vector<domain::InstalledPackage>,
                       application::ports::PackageCatalogError> override;

  [[nodiscard]] auto add(const domain::InstalledPackage& package)
      -> std::expected<void, application::ports::PackageCatalogError> override;

  [[nodiscard]] auto remove(const domain::PackageId& package_id)
      -> std::expected<void, application::ports::PackageCatalogError> override;

  [[nodiscard]] auto contains(const domain::PackageId& package_id) const
      -> std::expected<bool, application::ports::PackageCatalogError> override;

private:
  auto load() const -> std::expected<std::vector<domain::InstalledPackage>,
                                     application::ports::PackageCatalogError>;
  auto save(const std::vector<domain::InstalledPackage>& packages) const
      -> std::expected<void, application::ports::PackageCatalogError>;

  std::filesystem::path m_catalog_file;
  mutable std::mutex m_mutex;
};

} // namespace fmm::infrastructure
