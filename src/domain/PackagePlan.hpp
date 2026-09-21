#pragma once

#include "domain/PackageId.hpp"
#include "domain/PackageLocation.hpp"
#include "domain/PackageRootMapping.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace fmm::domain {

enum class PackageEntryKind : std::uint8_t { RegularFile, Directory, SymbolicLink };

class PackageLayoutEntry final {
public:
  PackageLayoutEntry(PackageRelativePath relative_path, PackageEntryKind kind)
      : m_relative_path(std::move(relative_path)), m_kind(kind) {}

  [[nodiscard]] auto relativePath() const noexcept -> const PackageRelativePath& {
    return m_relative_path;
  }

  [[nodiscard]] auto kind() const noexcept -> PackageEntryKind { return m_kind; }

  auto operator==(const PackageLayoutEntry&) const -> bool = default;

private:
  PackageRelativePath m_relative_path;
  PackageEntryKind m_kind;
};

class PackagePlan final {
public:
  PackagePlan(PackageId package_id, PackageLocation package_location,
              std::vector<PackageLayoutEntry> entries,
              std::vector<PackageRootMapping> root_mappings)
      : m_package_id(std::move(package_id)), m_package_location(std::move(package_location)),
        m_entries(std::move(entries)), m_root_mappings(std::move(root_mappings)) {}

  [[nodiscard]] auto packageId() const noexcept -> const PackageId& { return m_package_id; }

  [[nodiscard]] auto packageLocation() const noexcept -> const PackageLocation& {
    return m_package_location;
  }

  [[nodiscard]] auto entries() const noexcept -> const std::vector<PackageLayoutEntry>& {
    return m_entries;
  }

  [[nodiscard]] auto rootMappings() const noexcept -> const std::vector<PackageRootMapping>& {
    return m_root_mappings;
  }

  auto operator==(const PackagePlan&) const -> bool = default;

private:
  PackageId m_package_id;
  PackageLocation m_package_location;
  std::vector<PackageLayoutEntry> m_entries;
  std::vector<PackageRootMapping> m_root_mappings;
};

} // namespace fmm::domain
