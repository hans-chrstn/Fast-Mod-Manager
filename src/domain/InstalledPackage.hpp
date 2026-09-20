#pragma once

#include "domain/PackageId.hpp"
#include "domain/PackageLocation.hpp"
#include "domain/PackageMetadata.hpp"

namespace fmm::domain {

class InstalledPackage {
public:
  InstalledPackage(PackageId package_id, PackageMetadata metadata, PackageLocation location)
      : m_package_id(std::move(package_id)), m_metadata(std::move(metadata)),
        m_location(std::move(location)) {}

  [[nodiscard]] auto id() const -> const PackageId& { return m_package_id; }
  [[nodiscard]] auto metadata() const -> const PackageMetadata& { return m_metadata; }
  [[nodiscard]] auto location() const -> const PackageLocation& { return m_location; }

  auto operator==(const InstalledPackage&) const -> bool = default;

private:
  PackageId m_package_id;
  PackageMetadata m_metadata;
  PackageLocation m_location;
};

} // namespace fmm::domain
