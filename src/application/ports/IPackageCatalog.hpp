#pragma once

#include "domain/InstalledPackage.hpp"
#include "domain/PackageId.hpp"

#include <cstdint>
#include <expected>
#include <vector>

namespace fmm::application::ports {

enum class PackageCatalogError : std::uint8_t {
  AlreadyExists,
  NotFound,
  InvalidFormat,
  UnsupportedSchemaVersion,
  IoError
};

class IPackageCatalog {
public:
  virtual ~IPackageCatalog() = default;

  [[nodiscard]] virtual auto get(const domain::PackageId& package_id) const
      -> std::expected<domain::InstalledPackage, PackageCatalogError> = 0;

  [[nodiscard]] virtual auto list() const
      -> std::expected<std::vector<domain::InstalledPackage>, PackageCatalogError> = 0;

  [[nodiscard]] virtual auto add(const domain::InstalledPackage& package)
      -> std::expected<void, PackageCatalogError> = 0;

  [[nodiscard]] virtual auto remove(const domain::PackageId& package_id)
      -> std::expected<void, PackageCatalogError> = 0;

  [[nodiscard]] virtual auto contains(const domain::PackageId& package_id) const
      -> std::expected<bool, PackageCatalogError> = 0;
};

} // namespace fmm::application::ports
