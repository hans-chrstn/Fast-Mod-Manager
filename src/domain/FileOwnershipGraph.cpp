#include "domain/FileOwnershipGraph.hpp"

#include <algorithm>
#include <map>
#include <string>
#include <string_view>

namespace fmm::domain {
namespace {

auto bytesLess(std::string_view left, std::string_view right) -> bool {
  return std::ranges::lexicographical_compare(
      left, right, [](char left_byte, char right_byte) -> bool {
        return static_cast<unsigned char>(left_byte) < static_cast<unsigned char>(right_byte);
      });
}

auto containsNonDirectory(const FileOwnershipNode& node) -> bool {
  return std::ranges::any_of(node.claims(), [](const PackagePathClaim& claim) -> bool {
    return claim.kind() != PackageEntryKind::Directory;
  });
}

auto buildObstructions(const std::vector<FileOwnershipNode>& nodes)
    -> std::vector<PathObstruction> {
  std::vector<PathObstruction> obstructions;

  for (const auto& ancestor : nodes) {
    if (!containsNonDirectory(ancestor)) {
      continue;
    }

    const auto& ancestor_bytes = ancestor.path().nativeBytes();
    if (ancestor_bytes.empty()) {
      for (const auto& descendant : nodes) {
        if (!descendant.path().nativeBytes().empty()) {
          obstructions.emplace_back(ancestor.path(), descendant.path());
        }
      }
      continue;
    }

    auto descendant_prefix = ancestor_bytes;
    descendant_prefix += '/';
    auto descendant = std::ranges::lower_bound(
        nodes, std::string_view{descendant_prefix},
        [](std::string_view left, std::string_view right) -> bool {
          return bytesLess(left, right);
        },
        [](const FileOwnershipNode& node) -> std::string_view {
          return node.path().nativeBytes();
        });
    while (descendant != nodes.end() &&
           descendant->path().nativeBytes().starts_with(descendant_prefix)) {
      obstructions.emplace_back(ancestor.path(), descendant->path());
      ++descendant;
    }
  }

  return obstructions;
}

} // namespace

auto FileOwnershipGraph::create(const PackageDependencyGraph& package_graph,
                                std::vector<PackagePathClaim> claims)
    -> std::expected<FileOwnershipGraph, FileOwnershipGraphError> {
  const auto& package_order = package_graph.resolvedOrder();
  std::map<PackageId, std::size_t> package_ranks;
  for (std::size_t index = 0; index < package_order.size(); ++index) {
    package_ranks.emplace(package_order.at(index), index);
  }

  for (std::size_t index = 0; index < claims.size(); ++index) {
    if (!package_ranks.contains(claims.at(index).packageId())) {
      return std::unexpected(
          FileOwnershipGraphError{FileOwnershipGraphErrorCode::ClaimPackageNotEnabled,
                                  claims.at(index).packageId(), index});
    }
  }

  std::vector<std::size_t> ordered_claim_indexes(claims.size());
  for (std::size_t index = 0; index < ordered_claim_indexes.size(); ++index) {
    ordered_claim_indexes.at(index) = index;
  }
  std::ranges::stable_sort(ordered_claim_indexes,
                           [&claims, &package_ranks](std::size_t left, std::size_t right) -> bool {
                             const auto& left_claim = claims.at(left);
                             const auto& right_claim = claims.at(right);
                             const auto& left_path = left_claim.gameRelativePath().nativeBytes();
                             const auto& right_path = right_claim.gameRelativePath().nativeBytes();
                             if (left_path != right_path) {
                               return bytesLess(left_path, right_path);
                             }

                             const auto left_rank = package_ranks.at(left_claim.packageId());
                             const auto right_rank = package_ranks.at(right_claim.packageId());
                             if (left_rank != right_rank) {
                               return left_rank < right_rank;
                             }
                             return left < right;
                           });

  std::vector<FileOwnershipNode> nodes;
  std::vector<GameRelativePath> exact_collision_paths;
  for (std::size_t cursor = 0; cursor < ordered_claim_indexes.size();) {
    const auto& path = claims.at(ordered_claim_indexes.at(cursor)).gameRelativePath();
    std::vector<PackagePathClaim> node_claims;
    while (cursor < ordered_claim_indexes.size() &&
           claims.at(ordered_claim_indexes.at(cursor)).gameRelativePath() == path) {
      node_claims.push_back(claims.at(ordered_claim_indexes.at(cursor)));
      ++cursor;
    }

    const auto is_exact_collision =
        node_claims.size() > 1 &&
        std::ranges::any_of(node_claims, [](const PackagePathClaim& claim) -> bool {
          return claim.kind() != PackageEntryKind::Directory;
        });
    nodes.emplace_back(path, std::move(node_claims));
    if (is_exact_collision) {
      exact_collision_paths.push_back(path);
    }
  }

  auto obstructions = buildObstructions(nodes);
  return FileOwnershipGraph(std::vector<PackageId>{package_order}, std::move(claims),
                            std::move(nodes), std::move(exact_collision_paths),
                            std::move(obstructions));
}

} // namespace fmm::domain
