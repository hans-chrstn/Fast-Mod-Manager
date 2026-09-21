#pragma once

#include "domain/ConflictResolution.hpp"

#include <cstdint>
#include <expected>
#include <optional>
#include <utility>
#include <vector>

namespace fmm::domain {

class NamespaceSourceReference final {
public:
  NamespaceSourceReference(PackageId package_id, PackageRelativePath package_relative_path,
                           PackageLocation package_location)
      : m_package_id(std::move(package_id)),
        m_package_relative_path(std::move(package_relative_path)),
        m_package_location(std::move(package_location)) {}

  [[nodiscard]] auto packageId() const noexcept -> const PackageId& { return m_package_id; }

  [[nodiscard]] auto packageRelativePath() const noexcept -> const PackageRelativePath& {
    return m_package_relative_path;
  }

  [[nodiscard]] auto packageLocation() const noexcept -> const PackageLocation& {
    return m_package_location;
  }

  auto operator==(const NamespaceSourceReference&) const -> bool = default;

private:
  PackageId m_package_id;
  PackageRelativePath m_package_relative_path;
  PackageLocation m_package_location;
};

class NamespacePackagePlanRecord final {
public:
  NamespacePackagePlanRecord(PackageId lookup_package_id, PackagePlan package_plan)
      : m_lookup_package_id(std::move(lookup_package_id)), m_package_plan(std::move(package_plan)) {
  }

  [[nodiscard]] auto lookupPackageId() const noexcept -> const PackageId& {
    return m_lookup_package_id;
  }

  [[nodiscard]] auto packagePlan() const noexcept -> const PackagePlan& { return m_package_plan; }

  auto operator==(const NamespacePackagePlanRecord&) const -> bool = default;

private:
  PackageId m_lookup_package_id;
  PackagePlan m_package_plan;
};

enum class CompiledNamespaceEntryOrigin : std::uint8_t {
  SingleClaim,
  MergedDirectories,
  PriorityWinner,
  SyntheticDirectory,
};

class CompiledNamespaceEntry final {
public:
  [[nodiscard]] auto path() const noexcept -> const GameRelativePath& { return m_path; }

  [[nodiscard]] auto kind() const noexcept -> PackageEntryKind { return m_kind; }

  [[nodiscard]] auto origin() const noexcept -> CompiledNamespaceEntryOrigin { return m_origin; }

  [[nodiscard]] auto selectedClaim() const noexcept -> const std::optional<PackagePathClaim>& {
    return m_selected_claim;
  }

  [[nodiscard]] auto sourceReference() const noexcept
      -> const std::optional<NamespaceSourceReference>& {
    return m_source_reference;
  }

  auto operator==(const CompiledNamespaceEntry&) const -> bool = default;

private:
  friend class CompiledNamespace;

  CompiledNamespaceEntry(GameRelativePath path, PackageEntryKind kind,
                         CompiledNamespaceEntryOrigin origin,
                         std::optional<PackagePathClaim> selected_claim,
                         std::optional<NamespaceSourceReference> source_reference = std::nullopt)
      : m_path(std::move(path)), m_kind(kind), m_origin(origin),
        m_selected_claim(std::move(selected_claim)),
        m_source_reference(std::move(source_reference)) {}

  GameRelativePath m_path;
  PackageEntryKind m_kind;
  CompiledNamespaceEntryOrigin m_origin;
  std::optional<PackagePathClaim> m_selected_claim;
  std::optional<NamespaceSourceReference> m_source_reference;
};

enum class NamespaceCompilationErrorCode : std::uint8_t {
  BlockingConflicts,
  InvalidResolvedDecision,
};

class NamespaceCompilationError final {
public:
  explicit NamespaceCompilationError(NamespaceCompilationErrorCode code) : m_code(code) {}

  [[nodiscard]] auto code() const noexcept -> NamespaceCompilationErrorCode { return m_code; }

  auto operator==(const NamespaceCompilationError&) const -> bool = default;

private:
  NamespaceCompilationErrorCode m_code;
};

enum class NamespaceSourceMappingErrorCode : std::uint8_t {
  MissingPackage,
  DuplicatePackage,
  MismatchedPackageId,
  MissingSourceEntry,
  InvalidEntryKind,
};

class NamespaceSourceMappingError final {
public:
  NamespaceSourceMappingError(NamespaceSourceMappingErrorCode code,
                              std::optional<PackageId> package_id,
                              std::optional<PackageId> related_package_id,
                              std::optional<GameRelativePath> entry_path,
                              std::optional<PackageRelativePath> package_relative_path)
      : m_code(code), m_package_id(std::move(package_id)),
        m_related_package_id(std::move(related_package_id)), m_entry_path(std::move(entry_path)),
        m_package_relative_path(std::move(package_relative_path)) {}

  [[nodiscard]] auto code() const noexcept -> NamespaceSourceMappingErrorCode { return m_code; }

  [[nodiscard]] auto packageId() const noexcept -> const std::optional<PackageId>& {
    return m_package_id;
  }

  [[nodiscard]] auto relatedPackageId() const noexcept -> const std::optional<PackageId>& {
    return m_related_package_id;
  }

  [[nodiscard]] auto entryPath() const noexcept -> const std::optional<GameRelativePath>& {
    return m_entry_path;
  }

  [[nodiscard]] auto packageRelativePath() const noexcept
      -> const std::optional<PackageRelativePath>& {
    return m_package_relative_path;
  }

  auto operator==(const NamespaceSourceMappingError&) const -> bool = default;

private:
  NamespaceSourceMappingErrorCode m_code;
  std::optional<PackageId> m_package_id;
  std::optional<PackageId> m_related_package_id;
  std::optional<GameRelativePath> m_entry_path;
  std::optional<PackageRelativePath> m_package_relative_path;
};

class SourceMappedNamespace final {
public:
  [[nodiscard]] auto entries() const noexcept -> const std::vector<CompiledNamespaceEntry>& {
    return m_entries;
  }

  auto operator==(const SourceMappedNamespace&) const -> bool = default;

private:
  friend class CompiledNamespace;

  explicit SourceMappedNamespace(std::vector<CompiledNamespaceEntry> entries)
      : m_entries(std::move(entries)) {}

  std::vector<CompiledNamespaceEntry> m_entries;
};

class CompiledNamespace final {
public:
  [[nodiscard]] static auto compile(const ConflictResolutionReport& report)
      -> std::expected<CompiledNamespace, NamespaceCompilationError>;

  [[nodiscard]] auto entries() const noexcept -> const std::vector<CompiledNamespaceEntry>& {
    return m_entries;
  }

  [[nodiscard]] auto
  mapSources(const std::vector<NamespacePackagePlanRecord>& package_snapshot) const
      -> std::expected<SourceMappedNamespace, NamespaceSourceMappingError>;

  auto operator==(const CompiledNamespace&) const -> bool = default;

private:
  explicit CompiledNamespace(std::vector<CompiledNamespaceEntry> entries)
      : m_entries(std::move(entries)) {}

  std::vector<CompiledNamespaceEntry> m_entries;
};

} // namespace fmm::domain
