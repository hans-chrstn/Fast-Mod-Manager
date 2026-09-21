#pragma once

#include "domain/ConflictResolution.hpp"

#include <cstdint>
#include <expected>
#include <optional>
#include <utility>
#include <vector>

namespace fmm::domain {

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

  auto operator==(const CompiledNamespaceEntry&) const -> bool = default;

private:
  friend class CompiledNamespace;

  CompiledNamespaceEntry(GameRelativePath path, PackageEntryKind kind,
                         CompiledNamespaceEntryOrigin origin,
                         std::optional<PackagePathClaim> selected_claim)
      : m_path(std::move(path)), m_kind(kind), m_origin(origin),
        m_selected_claim(std::move(selected_claim)) {}

  GameRelativePath m_path;
  PackageEntryKind m_kind;
  CompiledNamespaceEntryOrigin m_origin;
  std::optional<PackagePathClaim> m_selected_claim;
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

class CompiledNamespace final {
public:
  [[nodiscard]] static auto compile(const ConflictResolutionReport& report)
      -> std::expected<CompiledNamespace, NamespaceCompilationError>;

  [[nodiscard]] auto entries() const noexcept -> const std::vector<CompiledNamespaceEntry>& {
    return m_entries;
  }

  auto operator==(const CompiledNamespace&) const -> bool = default;

private:
  explicit CompiledNamespace(std::vector<CompiledNamespaceEntry> entries)
      : m_entries(std::move(entries)) {}

  std::vector<CompiledNamespaceEntry> m_entries;
};

} // namespace fmm::domain
