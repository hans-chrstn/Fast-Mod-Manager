#include "domain/PackageDependencyGraph.hpp"
#include "domain/PluginId.hpp"

#include <catch2/catch_test_macros.hpp>
#include <expected>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace fmm::domain;

static_assert(std::is_constructible_v<PackageDependency, PackageId, PackageId>);
static_assert(!std::is_constructible_v<PackageDependency, PluginId, PackageId>);
static_assert(std::is_constructible_v<PackageOrderConstraint, PackageId, PackageId>);
static_assert(!std::is_constructible_v<PackageOrderConstraint, PackageId, PluginId>);
static_assert(std::is_same_v<decltype(PackageDependencyGraph::create({}, {}, {})),
                             std::expected<PackageDependencyGraph, PackageGraphError>>);
static_assert(
    std::is_same_v<decltype(std::declval<const PackageDependencyGraph&>().enabledPackages()),
                   const std::vector<PackageId>&>);
static_assert(std::is_same_v<decltype(std::declval<const PackageDependencyGraph&>().dependencies()),
                             const std::vector<PackageDependency>&>);
static_assert(
    std::is_same_v<decltype(std::declval<const PackageDependencyGraph&>().orderingConstraints()),
                   const std::vector<PackageOrderConstraint>&>);
static_assert(
    std::is_same_v<decltype(std::declval<const PackageDependencyGraph&>().resolvedOrder()),
                   const std::vector<PackageId>&>);

namespace {

auto requireError(std::expected<PackageDependencyGraph, PackageGraphError> result,
                  PackageGraphErrorCode code, const std::vector<PackageId>& package_ids) -> void {
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error().code() == code);
  REQUIRE(result.error().packageIds() == package_ids);
}

} // namespace

TEST_CASE("Package dependency graph supports empty and singleton inputs",
          "[domain][package-graph]") {
  const auto empty = PackageDependencyGraph::create({}, {}, {});
  REQUIRE(empty.has_value());
  REQUIRE(empty->enabledPackages().empty());
  REQUIRE(empty->dependencies().empty());
  REQUIRE(empty->orderingConstraints().empty());
  REQUIRE(empty->resolvedOrder().empty());

  const auto singleton = PackageDependencyGraph::create({PackageId{"only"}}, {}, {});
  REQUIRE(singleton.has_value());
  REQUIRE(singleton->resolvedOrder() == std::vector<PackageId>{PackageId{"only"}});
}

TEST_CASE("Package dependency graph preserves unconstrained enabled order",
          "[domain][package-graph]") {
  const std::vector<PackageId> enabled{PackageId{"package-c"}, PackageId{"package-a"},
                                       PackageId{"package-b"}};
  const auto graph = PackageDependencyGraph::create(enabled, {}, {});

  REQUIRE(graph.has_value());
  REQUIRE(graph->enabledPackages() == enabled);
  REQUIRE(graph->resolvedOrder() == enabled);
}

TEST_CASE("Package dependency graph owns its input snapshot", "[domain][package-graph]") {
  std::vector<PackageId> enabled{PackageId{"before"}, PackageId{"after"}};
  std::vector<PackageDependency> dependencies{
      PackageDependency{PackageId{"after"}, PackageId{"before"}},
  };
  std::vector<PackageOrderConstraint> ordering{
      PackageOrderConstraint{PackageId{"before"}, PackageId{"after"}},
  };
  const auto graph = PackageDependencyGraph::create(enabled, dependencies, ordering);

  enabled.clear();
  dependencies.clear();
  ordering.clear();

  REQUIRE(graph.has_value());
  REQUIRE(graph->enabledPackages() ==
          std::vector<PackageId>{PackageId{"before"}, PackageId{"after"}});
  REQUIRE(graph->dependencies() == std::vector<PackageDependency>{
                                       PackageDependency{PackageId{"after"}, PackageId{"before"}}});
  REQUIRE(graph->orderingConstraints() ==
          std::vector<PackageOrderConstraint>{
              PackageOrderConstraint{PackageId{"before"}, PackageId{"after"}}});
}

TEST_CASE("Package dependency graph resolves dependency and ordering chains",
          "[domain][package-graph]") {
  const std::vector<PackageDependency> dependencies{
      PackageDependency{PackageId{"application"}, PackageId{"library"}},
      PackageDependency{PackageId{"library"}, PackageId{"runtime"}},
  };
  const std::vector<PackageOrderConstraint> ordering{
      PackageOrderConstraint{PackageId{"bootstrap"}, PackageId{"runtime"}},
  };
  const auto graph = PackageDependencyGraph::create({PackageId{"application"}, PackageId{"library"},
                                                     PackageId{"runtime"}, PackageId{"bootstrap"}},
                                                    dependencies, ordering);

  REQUIRE(graph.has_value());
  REQUIRE(graph->dependencies() == dependencies);
  REQUIRE(graph->orderingConstraints() == ordering);
  REQUIRE(graph->resolvedOrder() ==
          std::vector<PackageId>{PackageId{"bootstrap"}, PackageId{"runtime"}, PackageId{"library"},
                                 PackageId{"application"}});
}

TEST_CASE("Package graph edges override the enabled-order tie-break", "[domain][package-graph]") {
  const auto graph = PackageDependencyGraph::create(
      {PackageId{"dependent"}, PackageId{"free"}, PackageId{"required"}},
      {PackageDependency{PackageId{"dependent"}, PackageId{"required"}}}, {});

  REQUIRE(graph.has_value());
  REQUIRE(graph->resolvedOrder() ==
          std::vector<PackageId>{PackageId{"free"}, PackageId{"required"}, PackageId{"dependent"}});
}

TEST_CASE("Package graph repeated effective edges are idempotent", "[domain][package-graph]") {
  const std::vector<PackageDependency> dependencies{
      PackageDependency{PackageId{"after"}, PackageId{"before"}},
      PackageDependency{PackageId{"after"}, PackageId{"before"}},
  };
  const std::vector<PackageOrderConstraint> ordering{
      PackageOrderConstraint{PackageId{"before"}, PackageId{"after"}},
  };
  const auto graph = PackageDependencyGraph::create(
      {PackageId{"after"}, PackageId{"before"}, PackageId{"independent"}}, dependencies, ordering);

  REQUIRE(graph.has_value());
  REQUIRE(graph->dependencies() == dependencies);
  REQUIRE(graph->orderingConstraints() == ordering);
  REQUIRE(graph->resolvedOrder() == std::vector<PackageId>{PackageId{"before"}, PackageId{"after"},
                                                           PackageId{"independent"}});
}

TEST_CASE("Package graph order is deterministic when multiple packages become ready",
          "[domain][package-graph]") {
  const std::vector<PackageId> enabled{PackageId{"last"}, PackageId{"first"}, PackageId{"middle"},
                                       PackageId{"root"}};
  const std::vector<PackageDependency> dependencies{
      PackageDependency{PackageId{"last"}, PackageId{"root"}},
      PackageDependency{PackageId{"first"}, PackageId{"root"}},
      PackageDependency{PackageId{"middle"}, PackageId{"root"}},
  };

  const auto first = PackageDependencyGraph::create(enabled, dependencies, {});
  const auto second = PackageDependencyGraph::create(enabled, dependencies, {});

  REQUIRE(first.has_value());
  REQUIRE(second.has_value());
  REQUIRE(first == second);
  REQUIRE(first->resolvedOrder() == std::vector<PackageId>{PackageId{"root"}, PackageId{"last"},
                                                           PackageId{"first"},
                                                           PackageId{"middle"}});
}

TEST_CASE("Package graph rejects duplicate enabled IDs", "[domain][package-graph]") {
  requireError(
      PackageDependencyGraph::create({PackageId{"duplicate"}, PackageId{"duplicate"}}, {}, {}),
      PackageGraphErrorCode::DuplicateEnabledPackage,
      std::vector<PackageId>{PackageId{"duplicate"}});
}

TEST_CASE("Package graph rejects non-enabled dependency endpoints", "[domain][package-graph]") {
  SECTION("dependent") {
    requireError(PackageDependencyGraph::create(
                     {PackageId{"enabled"}},
                     {PackageDependency{PackageId{"missing"}, PackageId{"enabled"}}}, {}),
                 PackageGraphErrorCode::DependentPackageNotEnabled,
                 std::vector<PackageId>{PackageId{"missing"}});
  }

  SECTION("required") {
    requireError(PackageDependencyGraph::create(
                     {PackageId{"enabled"}},
                     {PackageDependency{PackageId{"enabled"}, PackageId{"missing"}}}, {}),
                 PackageGraphErrorCode::RequiredPackageNotEnabled,
                 std::vector<PackageId>{PackageId{"missing"}});
  }
}

TEST_CASE("Package graph reports the first invalid declaration deterministically",
          "[domain][package-graph]") {
  requireError(PackageDependencyGraph::create(
                   {PackageId{"enabled"}},
                   {PackageDependency{PackageId{"first-missing"}, PackageId{"enabled"}},
                    PackageDependency{PackageId{"second-missing"}, PackageId{"enabled"}}},
                   {PackageOrderConstraint{PackageId{"third-missing"}, PackageId{"enabled"}}}),
               PackageGraphErrorCode::DependentPackageNotEnabled,
               std::vector<PackageId>{PackageId{"first-missing"}});
}

TEST_CASE("Package graph rejects non-enabled ordering endpoints", "[domain][package-graph]") {
  SECTION("before") {
    requireError(PackageDependencyGraph::create(
                     {PackageId{"enabled"}}, {},
                     {PackageOrderConstraint{PackageId{"missing"}, PackageId{"enabled"}}}),
                 PackageGraphErrorCode::OrderingPackageNotEnabled,
                 std::vector<PackageId>{PackageId{"missing"}});
  }

  SECTION("after") {
    requireError(PackageDependencyGraph::create(
                     {PackageId{"enabled"}}, {},
                     {PackageOrderConstraint{PackageId{"enabled"}, PackageId{"missing"}}}),
                 PackageGraphErrorCode::OrderingPackageNotEnabled,
                 std::vector<PackageId>{PackageId{"missing"}});
  }
}

TEST_CASE("Package graph rejects dependency and ordering self-relations",
          "[domain][package-graph]") {
  SECTION("dependency") {
    requireError(
        PackageDependencyGraph::create(
            {PackageId{"same"}}, {PackageDependency{PackageId{"same"}, PackageId{"same"}}}, {}),
        PackageGraphErrorCode::SelfDependency, std::vector<PackageId>{PackageId{"same"}});
  }

  SECTION("ordering") {
    requireError(PackageDependencyGraph::create(
                     {PackageId{"same"}}, {},
                     {PackageOrderConstraint{PackageId{"same"}, PackageId{"same"}}}),
                 PackageGraphErrorCode::SelfOrdering, std::vector<PackageId>{PackageId{"same"}});
  }
}

TEST_CASE("Package graph reports deterministic closed dependency cycles",
          "[domain][package-graph]") {
  const std::vector<PackageId> enabled{PackageId{"a"}, PackageId{"b"}, PackageId{"c"},
                                       PackageId{"downstream"}};
  const std::vector<PackageDependency> dependencies{
      PackageDependency{PackageId{"b"}, PackageId{"a"}},
      PackageDependency{PackageId{"c"}, PackageId{"b"}},
      PackageDependency{PackageId{"a"}, PackageId{"c"}},
      PackageDependency{PackageId{"downstream"}, PackageId{"c"}},
  };

  const auto first = PackageDependencyGraph::create(enabled, dependencies, {});
  const auto second = PackageDependencyGraph::create(enabled, dependencies, {});

  requireError(first, PackageGraphErrorCode::Cycle,
               {PackageId{"a"}, PackageId{"b"}, PackageId{"c"}, PackageId{"a"}});
  requireError(second, PackageGraphErrorCode::Cycle,
               {PackageId{"a"}, PackageId{"b"}, PackageId{"c"}, PackageId{"a"}});
}

TEST_CASE("Package graph detects pure ordering and mixed cycles", "[domain][package-graph]") {
  SECTION("ordering") {
    requireError(
        PackageDependencyGraph::create({PackageId{"a"}, PackageId{"b"}}, {},
                                       {PackageOrderConstraint{PackageId{"a"}, PackageId{"b"}},
                                        PackageOrderConstraint{PackageId{"b"}, PackageId{"a"}}}),
        PackageGraphErrorCode::Cycle, {PackageId{"a"}, PackageId{"b"}, PackageId{"a"}});
  }

  SECTION("mixed") {
    requireError(
        PackageDependencyGraph::create({PackageId{"a"}, PackageId{"b"}, PackageId{"c"}},
                                       {PackageDependency{PackageId{"b"}, PackageId{"a"}}},
                                       {PackageOrderConstraint{PackageId{"b"}, PackageId{"c"}},
                                        PackageOrderConstraint{PackageId{"c"}, PackageId{"a"}}}),
        PackageGraphErrorCode::Cycle,
        {PackageId{"a"}, PackageId{"b"}, PackageId{"c"}, PackageId{"a"}});
  }
}
