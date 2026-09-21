#include "domain/ConflictResolution.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <string_view>

namespace fmm::domain {
namespace {

auto appendClaims(std::vector<PackagePathClaim>& destination, const FileOwnershipNode& node)
    -> void {
  destination.insert(destination.end(), node.claims().begin(), node.claims().end());
}

} // namespace

auto ConflictResolutionPolicy::create(const FileOwnershipGraph& graph,
                                      std::vector<PackageId> priority_order)
    -> std::expected<ConflictResolutionPolicy, ConflictPolicyError> {
  const std::set<PackageId> graph_packages(graph.packageOrder().begin(),
                                           graph.packageOrder().end());
  std::set<PackageId> seen_packages;

  for (std::size_t index = 0; index < priority_order.size(); ++index) {
    const auto& package_id = priority_order.at(index);
    if (seen_packages.contains(package_id)) {
      return std::unexpected(
          ConflictPolicyError{ConflictPolicyErrorCode::DuplicatePackage, package_id, index});
    }
    if (!graph_packages.contains(package_id)) {
      return std::unexpected(
          ConflictPolicyError{ConflictPolicyErrorCode::UnknownPackage, package_id, index});
    }
    seen_packages.insert(package_id);
  }

  const auto missing_package = std::ranges::find_if(
      graph.packageOrder(), [&seen_packages](const PackageId& package_id) -> bool {
        return !seen_packages.contains(package_id);
      });
  if (missing_package != graph.packageOrder().end()) {
    return std::unexpected(ConflictPolicyError{ConflictPolicyErrorCode::MissingPackage,
                                               *missing_package, std::nullopt});
  }

  return ConflictResolutionPolicy{std::move(priority_order)};
}

auto ConflictResolutionReport::create(const FileOwnershipGraph& graph,
                                      std::vector<PackageId> priority_order)
    -> std::expected<ConflictResolutionReport, ConflictPolicyError> {
  auto policy = ConflictResolutionPolicy::create(graph, std::move(priority_order));
  if (!policy.has_value()) {
    return std::unexpected(policy.error());
  }

  std::map<PackageId, std::size_t> package_ranks;
  for (std::size_t index = 0; index < policy->priorityOrder().size(); ++index) {
    package_ranks.emplace(policy->priorityOrder().at(index), index);
  }

  std::vector<ExactPathDecision> decisions;
  decisions.reserve(graph.nodes().size());
  std::vector<ConflictDiagnostic> diagnostics;
  bool has_blocking_conflicts = false;

  for (const auto& node : graph.nodes()) {
    if (node.claims().size() == 1) {
      decisions.emplace_back(node.path(), ExactPathDecisionKind::SingleClaim, node.claims(),
                             node.claims().front());
      continue;
    }

    const auto all_directories =
        std::ranges::all_of(node.claims(), [](const PackagePathClaim& claim) -> bool {
          return claim.kind() == PackageEntryKind::Directory;
        });
    if (all_directories) {
      decisions.emplace_back(node.path(), ExactPathDecisionKind::MergedDirectories, node.claims(),
                             std::nullopt);
      continue;
    }

    const auto contains_directory =
        std::ranges::any_of(node.claims(), [](const PackagePathClaim& claim) -> bool {
          return claim.kind() == PackageEntryKind::Directory;
        });
    if (contains_directory) {
      decisions.emplace_back(node.path(), ExactPathDecisionKind::Unresolved, node.claims(),
                             std::nullopt);
      diagnostics.emplace_back(
          ConflictDiagnosticCode::MixedEntryKinds, ConflictDiagnosticSeverity::Error, node.path(),
          std::nullopt, node.claims(), std::nullopt, ConflictRemediation::CorrectPackageMapping);
      has_blocking_conflicts = true;
      continue;
    }

    const PackagePathClaim* selected_claim = nullptr;
    std::size_t selected_rank = 0;
    std::size_t selected_package_claim_count = 0;
    for (const auto& claim : node.claims()) {
      const auto rank = package_ranks.at(claim.packageId());
      if (selected_claim == nullptr || rank > selected_rank) {
        selected_claim = &claim;
        selected_rank = rank;
        selected_package_claim_count = 1;
      } else if (rank == selected_rank) {
        ++selected_package_claim_count;
      }
    }

    if (selected_package_claim_count > 1) {
      decisions.emplace_back(node.path(), ExactPathDecisionKind::Unresolved, node.claims(),
                             std::nullopt);
      diagnostics.emplace_back(ConflictDiagnosticCode::AmbiguousHighestPriorityClaim,
                               ConflictDiagnosticSeverity::Error, node.path(), std::nullopt,
                               node.claims(), std::nullopt,
                               ConflictRemediation::CorrectPackageMapping);
      has_blocking_conflicts = true;
      continue;
    }

    decisions.emplace_back(node.path(), ExactPathDecisionKind::PriorityWinner, node.claims(),
                           *selected_claim);
    diagnostics.emplace_back(ConflictDiagnosticCode::ExactPathWinnerSelected,
                             ConflictDiagnosticSeverity::Warning, node.path(), std::nullopt,
                             node.claims(), *selected_claim,
                             ConflictRemediation::ReviewPackagePriority);
  }

  std::map<std::string_view, const FileOwnershipNode*, std::less<>> nodes_by_path;
  for (const auto& node : graph.nodes()) {
    nodes_by_path.emplace(node.path().nativeBytes(), &node);
  }

  for (const auto& obstruction : graph.obstructions()) {
    const auto* ancestor = nodes_by_path.at(obstruction.ancestorPath().nativeBytes());
    const auto* descendant = nodes_by_path.at(obstruction.descendantPath().nativeBytes());
    std::vector<PackagePathClaim> claims;
    claims.reserve(ancestor->claims().size() + descendant->claims().size());
    appendClaims(claims, *ancestor);
    appendClaims(claims, *descendant);
    diagnostics.emplace_back(ConflictDiagnosticCode::PathObstruction,
                             ConflictDiagnosticSeverity::Error, obstruction.ancestorPath(),
                             obstruction.descendantPath(), std::move(claims), std::nullopt,
                             ConflictRemediation::CorrectPackageLayout);
    has_blocking_conflicts = true;
  }

  return ConflictResolutionReport{std::move(*policy), std::move(decisions), std::move(diagnostics),
                                  has_blocking_conflicts};
}

} // namespace fmm::domain
