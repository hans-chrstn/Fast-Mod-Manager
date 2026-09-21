#include "domain/PackageDependencyGraph.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <optional>
#include <set>

namespace fmm::domain {
namespace {

using PackageIndex = std::size_t;
using PackageIndexes = std::map<PackageId, PackageIndex>;
using AdjacencyList = std::vector<std::vector<PackageIndex>>;

enum class VisitState : std::uint8_t { Unvisited, Visiting, Visited };

struct VisitFrame {
  PackageIndex package;
  PackageIndex next_child;
};

auto addEdge(PackageIndex source, PackageIndex target,
             std::set<std::pair<PackageIndex, PackageIndex>>& edges, AdjacencyList& adjacency,
             std::vector<PackageIndex>& indegrees) -> void {
  if (edges.emplace(source, target).second) {
    adjacency.at(source).push_back(target);
    ++indegrees.at(target);
  }
}

auto findCycle(const std::vector<PackageId>& enabled_packages, const AdjacencyList& adjacency)
    -> std::optional<std::vector<PackageId>> {
  const auto no_position = std::numeric_limits<PackageIndex>::max();
  std::vector<VisitState> states(enabled_packages.size(), VisitState::Unvisited);
  std::vector<PackageIndex> positions(enabled_packages.size(), no_position);
  std::vector<PackageIndex> path;
  std::vector<VisitFrame> stack;

  for (PackageIndex start = 0; start < enabled_packages.size(); ++start) {
    if (states.at(start) != VisitState::Unvisited) {
      continue;
    }

    states.at(start) = VisitState::Visiting;
    positions.at(start) = path.size();
    path.push_back(start);
    stack.push_back(VisitFrame{.package = start, .next_child = 0});

    while (!stack.empty()) {
      auto& frame = stack.back();
      const auto& children = adjacency.at(frame.package);
      if (frame.next_child == children.size()) {
        states.at(frame.package) = VisitState::Visited;
        positions.at(frame.package) = no_position;
        path.pop_back();
        stack.pop_back();
        continue;
      }

      const auto child = children.at(frame.next_child);
      ++frame.next_child;

      if (states.at(child) == VisitState::Unvisited) {
        states.at(child) = VisitState::Visiting;
        positions.at(child) = path.size();
        path.push_back(child);
        stack.push_back(VisitFrame{.package = child, .next_child = 0});
        continue;
      }

      if (states.at(child) == VisitState::Visiting) {
        std::vector<PackageId> cycle;
        const auto cycle_start = positions.at(child);
        cycle.reserve(path.size() - cycle_start + 1);
        for (auto position = cycle_start; position < path.size(); ++position) {
          cycle.push_back(enabled_packages.at(path.at(position)));
        }
        cycle.push_back(enabled_packages.at(child));
        return cycle;
      }
    }
  }

  return std::nullopt;
}

auto resolveOrder(const std::vector<PackageId>& enabled_packages, const AdjacencyList& adjacency,
                  std::vector<PackageIndex> indegrees) -> std::vector<PackageId> {
  std::set<PackageIndex> ready;
  for (PackageIndex index = 0; index < indegrees.size(); ++index) {
    if (indegrees.at(index) == 0) {
      ready.insert(index);
    }
  }

  std::vector<PackageId> resolved_order;
  resolved_order.reserve(enabled_packages.size());
  while (!ready.empty()) {
    const auto package = *ready.begin();
    ready.erase(ready.begin());
    resolved_order.push_back(enabled_packages.at(package));

    for (const auto dependent : adjacency.at(package)) {
      auto& indegree = indegrees.at(dependent);
      --indegree;
      if (indegree == 0) {
        ready.insert(dependent);
      }
    }
  }

  return resolved_order;
}

} // namespace

auto PackageDependencyGraph::create(std::vector<PackageId> enabled_packages,
                                    std::vector<PackageDependency> dependencies,
                                    std::vector<PackageOrderConstraint> ordering_constraints)
    -> std::expected<PackageDependencyGraph, PackageGraphError> {
  PackageIndexes indexes;
  for (PackageIndex index = 0; index < enabled_packages.size(); ++index) {
    const auto& package = enabled_packages.at(index);
    if (!indexes.emplace(package, index).second) {
      return std::unexpected(PackageGraphError{PackageGraphErrorCode::DuplicateEnabledPackage,
                                               std::vector<PackageId>{package}});
    }
  }

  AdjacencyList adjacency(enabled_packages.size());
  std::vector<PackageIndex> indegrees(enabled_packages.size(), 0);
  std::set<std::pair<PackageIndex, PackageIndex>> edges;

  for (const auto& dependency : dependencies) {
    const auto dependent = indexes.find(dependency.dependentPackage());
    if (dependent == indexes.end()) {
      return std::unexpected(
          PackageGraphError{PackageGraphErrorCode::DependentPackageNotEnabled,
                            std::vector<PackageId>{dependency.dependentPackage()}});
    }

    const auto required = indexes.find(dependency.requiredPackage());
    if (required == indexes.end()) {
      return std::unexpected(
          PackageGraphError{PackageGraphErrorCode::RequiredPackageNotEnabled,
                            std::vector<PackageId>{dependency.requiredPackage()}});
    }

    if (dependent->second == required->second) {
      return std::unexpected(PackageGraphError{PackageGraphErrorCode::SelfDependency,
                                               std::vector<PackageId>{dependent->first}});
    }

    addEdge(required->second, dependent->second, edges, adjacency, indegrees);
  }

  for (const auto& constraint : ordering_constraints) {
    const auto before = indexes.find(constraint.beforePackage());
    if (before == indexes.end()) {
      return std::unexpected(PackageGraphError{PackageGraphErrorCode::OrderingPackageNotEnabled,
                                               std::vector<PackageId>{constraint.beforePackage()}});
    }

    const auto after = indexes.find(constraint.afterPackage());
    if (after == indexes.end()) {
      return std::unexpected(PackageGraphError{PackageGraphErrorCode::OrderingPackageNotEnabled,
                                               std::vector<PackageId>{constraint.afterPackage()}});
    }

    if (before->second == after->second) {
      return std::unexpected(PackageGraphError{PackageGraphErrorCode::SelfOrdering,
                                               std::vector<PackageId>{before->first}});
    }

    addEdge(before->second, after->second, edges, adjacency, indegrees);
  }

  for (auto& children : adjacency) {
    std::ranges::sort(children);
  }

  if (auto cycle = findCycle(enabled_packages, adjacency); cycle.has_value()) {
    return std::unexpected(
        PackageGraphError{PackageGraphErrorCode::Cycle, std::move(cycle).value()});
  }

  auto resolved_order = resolveOrder(enabled_packages, adjacency, std::move(indegrees));
  return PackageDependencyGraph(std::move(enabled_packages), std::move(dependencies),
                                std::move(ordering_constraints), std::move(resolved_order));
}

} // namespace fmm::domain
