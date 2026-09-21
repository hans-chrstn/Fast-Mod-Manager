#pragma once

#include "domain/PackageId.hpp"

#include <cstdint>
#include <expected>
#include <utility>
#include <vector>

namespace fmm::domain {

class PackageDependency final {
public:
  PackageDependency(PackageId dependent_package, PackageId required_package)
      : m_dependent_package(std::move(dependent_package)),
        m_required_package(std::move(required_package)) {}

  [[nodiscard]] auto dependentPackage() const noexcept -> const PackageId& {
    return m_dependent_package;
  }

  [[nodiscard]] auto requiredPackage() const noexcept -> const PackageId& {
    return m_required_package;
  }

  auto operator==(const PackageDependency&) const -> bool = default;

private:
  PackageId m_dependent_package;
  PackageId m_required_package;
};

class PackageOrderConstraint final {
public:
  PackageOrderConstraint(PackageId before_package, PackageId after_package)
      : m_before_package(std::move(before_package)), m_after_package(std::move(after_package)) {}

  [[nodiscard]] auto beforePackage() const noexcept -> const PackageId& { return m_before_package; }

  [[nodiscard]] auto afterPackage() const noexcept -> const PackageId& { return m_after_package; }

  auto operator==(const PackageOrderConstraint&) const -> bool = default;

private:
  PackageId m_before_package;
  PackageId m_after_package;
};

enum class PackageGraphErrorCode : std::uint8_t {
  DuplicateEnabledPackage,
  DependentPackageNotEnabled,
  RequiredPackageNotEnabled,
  OrderingPackageNotEnabled,
  SelfDependency,
  SelfOrdering,
  Cycle,
};

class PackageGraphError final {
public:
  PackageGraphError(PackageGraphErrorCode code, std::vector<PackageId> package_ids)
      : m_code(code), m_package_ids(std::move(package_ids)) {}

  [[nodiscard]] auto code() const noexcept -> PackageGraphErrorCode { return m_code; }

  [[nodiscard]] auto packageIds() const noexcept -> const std::vector<PackageId>& {
    return m_package_ids;
  }

  auto operator==(const PackageGraphError&) const -> bool = default;

private:
  PackageGraphErrorCode m_code;
  std::vector<PackageId> m_package_ids;
};

class PackageDependencyGraph final {
public:
  [[nodiscard]] static auto create(std::vector<PackageId> enabled_packages,
                                   std::vector<PackageDependency> dependencies,
                                   std::vector<PackageOrderConstraint> ordering_constraints)
      -> std::expected<PackageDependencyGraph, PackageGraphError>;

  [[nodiscard]] auto enabledPackages() const noexcept -> const std::vector<PackageId>& {
    return m_enabled_packages;
  }

  [[nodiscard]] auto dependencies() const noexcept -> const std::vector<PackageDependency>& {
    return m_dependencies;
  }

  [[nodiscard]] auto orderingConstraints() const noexcept
      -> const std::vector<PackageOrderConstraint>& {
    return m_ordering_constraints;
  }

  [[nodiscard]] auto resolvedOrder() const noexcept -> const std::vector<PackageId>& {
    return m_resolved_order;
  }

  auto operator==(const PackageDependencyGraph&) const -> bool = default;

private:
  PackageDependencyGraph(std::vector<PackageId> enabled_packages,
                         std::vector<PackageDependency> dependencies,
                         std::vector<PackageOrderConstraint> ordering_constraints,
                         std::vector<PackageId> resolved_order)
      : m_enabled_packages(std::move(enabled_packages)), m_dependencies(std::move(dependencies)),
        m_ordering_constraints(std::move(ordering_constraints)),
        m_resolved_order(std::move(resolved_order)) {}

  std::vector<PackageId> m_enabled_packages;
  std::vector<PackageDependency> m_dependencies;
  std::vector<PackageOrderConstraint> m_ordering_constraints;
  std::vector<PackageId> m_resolved_order;
};

} // namespace fmm::domain
