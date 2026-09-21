#pragma once

#include "domain/PackageId.hpp"
#include "domain/PackageLocation.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace fmm::domain {

enum class PackageEntryKind : std::uint8_t { RegularFile, Directory, SymbolicLink };

class PackageRelativePath final {
public:
  explicit PackageRelativePath(std::string native_bytes)
      : m_native_bytes(std::move(native_bytes)) {}

  [[nodiscard]] auto nativeBytes() const noexcept -> const std::string& { return m_native_bytes; }

  auto operator==(const PackageRelativePath&) const -> bool = default;

private:
  std::string m_native_bytes;
};

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
              std::vector<PackageLayoutEntry> entries)
      : m_package_id(std::move(package_id)), m_package_location(std::move(package_location)),
        m_entries(std::move(entries)) {}

  [[nodiscard]] auto packageId() const noexcept -> const PackageId& { return m_package_id; }

  [[nodiscard]] auto packageLocation() const noexcept -> const PackageLocation& {
    return m_package_location;
  }

  [[nodiscard]] auto entries() const noexcept -> const std::vector<PackageLayoutEntry>& {
    return m_entries;
  }

  auto operator==(const PackagePlan&) const -> bool = default;

private:
  PackageId m_package_id;
  PackageLocation m_package_location;
  std::vector<PackageLayoutEntry> m_entries;
};

} // namespace fmm::domain
