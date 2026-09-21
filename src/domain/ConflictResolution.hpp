#pragma once

#include "domain/FileOwnershipGraph.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <utility>
#include <vector>

namespace fmm::domain {

enum class ConflictPolicyErrorCode : std::uint8_t {
  DuplicatePackage,
  UnknownPackage,
  MissingPackage,
};

class ConflictPolicyError final {
public:
  ConflictPolicyError(ConflictPolicyErrorCode code, PackageId package_id,
                      std::optional<std::size_t> priority_index)
      : m_code(code), m_package_id(std::move(package_id)), m_priority_index(priority_index) {}

  [[nodiscard]] auto code() const noexcept -> ConflictPolicyErrorCode { return m_code; }

  [[nodiscard]] auto packageId() const noexcept -> const PackageId& { return m_package_id; }

  [[nodiscard]] auto priorityIndex() const noexcept -> std::optional<std::size_t> {
    return m_priority_index;
  }

  auto operator==(const ConflictPolicyError&) const -> bool = default;

private:
  ConflictPolicyErrorCode m_code;
  PackageId m_package_id;
  std::optional<std::size_t> m_priority_index;
};

class ConflictResolutionPolicy final {
public:
  [[nodiscard]] static auto create(const FileOwnershipGraph& graph,
                                   std::vector<PackageId> priority_order)
      -> std::expected<ConflictResolutionPolicy, ConflictPolicyError>;

  [[nodiscard]] auto priorityOrder() const noexcept -> const std::vector<PackageId>& {
    return m_priority_order;
  }

  auto operator==(const ConflictResolutionPolicy&) const -> bool = default;

private:
  explicit ConflictResolutionPolicy(std::vector<PackageId> priority_order)
      : m_priority_order(std::move(priority_order)) {}

  std::vector<PackageId> m_priority_order;
};

enum class ExactPathDecisionKind : std::uint8_t {
  SingleClaim,
  MergedDirectories,
  PriorityWinner,
  Unresolved,
};

class ExactPathDecision final {
public:
  ExactPathDecision(GameRelativePath path, ExactPathDecisionKind kind,
                    std::vector<PackagePathClaim> claims,
                    std::optional<PackagePathClaim> selected_claim)
      : m_path(std::move(path)), m_kind(kind), m_claims(std::move(claims)),
        m_selected_claim(std::move(selected_claim)) {}

  [[nodiscard]] auto path() const noexcept -> const GameRelativePath& { return m_path; }

  [[nodiscard]] auto kind() const noexcept -> ExactPathDecisionKind { return m_kind; }

  [[nodiscard]] auto claims() const noexcept -> const std::vector<PackagePathClaim>& {
    return m_claims;
  }

  [[nodiscard]] auto selectedClaim() const noexcept -> const std::optional<PackagePathClaim>& {
    return m_selected_claim;
  }

  auto operator==(const ExactPathDecision&) const -> bool = default;

private:
  GameRelativePath m_path;
  ExactPathDecisionKind m_kind;
  std::vector<PackagePathClaim> m_claims;
  std::optional<PackagePathClaim> m_selected_claim;
};

enum class ConflictDiagnosticSeverity : std::uint8_t { Warning, Error };

enum class ConflictDiagnosticCode : std::uint8_t {
  ExactPathWinnerSelected,
  MixedEntryKinds,
  AmbiguousHighestPriorityClaim,
  PathObstruction,
};

enum class ConflictRemediation : std::uint8_t {
  ReviewPackagePriority,
  CorrectPackageMapping,
  CorrectPackageLayout,
};

class ConflictDiagnostic final {
public:
  ConflictDiagnostic(ConflictDiagnosticCode code, ConflictDiagnosticSeverity severity,
                     GameRelativePath primary_path, std::optional<GameRelativePath> related_path,
                     std::vector<PackagePathClaim> claims,
                     std::optional<PackagePathClaim> selected_claim,
                     ConflictRemediation remediation)
      : m_code(code), m_severity(severity), m_primary_path(std::move(primary_path)),
        m_related_path(std::move(related_path)), m_claims(std::move(claims)),
        m_selected_claim(std::move(selected_claim)), m_remediation(remediation) {}

  [[nodiscard]] auto code() const noexcept -> ConflictDiagnosticCode { return m_code; }

  [[nodiscard]] auto severity() const noexcept -> ConflictDiagnosticSeverity { return m_severity; }

  [[nodiscard]] auto primaryPath() const noexcept -> const GameRelativePath& {
    return m_primary_path;
  }

  [[nodiscard]] auto relatedPath() const noexcept -> const std::optional<GameRelativePath>& {
    return m_related_path;
  }

  [[nodiscard]] auto claims() const noexcept -> const std::vector<PackagePathClaim>& {
    return m_claims;
  }

  [[nodiscard]] auto selectedClaim() const noexcept -> const std::optional<PackagePathClaim>& {
    return m_selected_claim;
  }

  [[nodiscard]] auto remediation() const noexcept -> ConflictRemediation { return m_remediation; }

  auto operator==(const ConflictDiagnostic&) const -> bool = default;

private:
  ConflictDiagnosticCode m_code;
  ConflictDiagnosticSeverity m_severity;
  GameRelativePath m_primary_path;
  std::optional<GameRelativePath> m_related_path;
  std::vector<PackagePathClaim> m_claims;
  std::optional<PackagePathClaim> m_selected_claim;
  ConflictRemediation m_remediation;
};

class ConflictResolutionReport final {
public:
  [[nodiscard]] static auto create(const FileOwnershipGraph& graph,
                                   std::vector<PackageId> priority_order)
      -> std::expected<ConflictResolutionReport, ConflictPolicyError>;

  [[nodiscard]] auto policy() const noexcept -> const ConflictResolutionPolicy& { return m_policy; }

  [[nodiscard]] auto decisions() const noexcept -> const std::vector<ExactPathDecision>& {
    return m_decisions;
  }

  [[nodiscard]] auto diagnostics() const noexcept -> const std::vector<ConflictDiagnostic>& {
    return m_diagnostics;
  }

  [[nodiscard]] auto hasBlockingConflicts() const noexcept -> bool {
    return m_has_blocking_conflicts;
  }

  auto operator==(const ConflictResolutionReport&) const -> bool = default;

private:
  ConflictResolutionReport(ConflictResolutionPolicy policy,
                           std::vector<ExactPathDecision> decisions,
                           std::vector<ConflictDiagnostic> diagnostics, bool has_blocking_conflicts)
      : m_policy(std::move(policy)), m_decisions(std::move(decisions)),
        m_diagnostics(std::move(diagnostics)), m_has_blocking_conflicts(has_blocking_conflicts) {}

  ConflictResolutionPolicy m_policy;
  std::vector<ExactPathDecision> m_decisions;
  std::vector<ConflictDiagnostic> m_diagnostics;
  bool m_has_blocking_conflicts;
};

} // namespace fmm::domain
