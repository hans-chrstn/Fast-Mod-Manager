#include "domain/FileOwnershipGraph.hpp"
#include "domain/PackageRootMapping.hpp"

#include <catch2/catch_test_macros.hpp>
#include <expected>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace fmm::domain;

static_assert(std::is_constructible_v<PackagePathClaim, PackageId, PackageRelativePath,
                                      GameRelativePath, PackageEntryKind>);
static_assert(!std::is_constructible_v<PackagePathClaim, PackageId, GameRelativePath,
                                       PackageRelativePath, PackageEntryKind>);
static_assert(std::is_same_v<decltype(FileOwnershipGraph::create(
                                 std::declval<const PackageDependencyGraph&>(), {})),
                             std::expected<FileOwnershipGraph, FileOwnershipGraphError>>);
static_assert(
    std::is_same_v<decltype(std::declval<const PackagePathClaim&>().packageRelativePath()),
                   const PackageRelativePath&>);
static_assert(std::is_same_v<decltype(std::declval<const PackagePathClaim&>().gameRelativePath()),
                             const GameRelativePath&>);
static_assert(std::is_same_v<decltype(std::declval<const FileOwnershipGraph&>().packageOrder()),
                             const std::vector<PackageId>&>);
static_assert(std::is_same_v<decltype(std::declval<const FileOwnershipGraph&>().claims()),
                             const std::vector<PackagePathClaim>&>);
static_assert(std::is_same_v<decltype(std::declval<const FileOwnershipGraph&>().nodes()),
                             const std::vector<FileOwnershipNode>&>);
static_assert(
    std::is_same_v<decltype(std::declval<const FileOwnershipGraph&>().exactCollisionPaths()),
                   const std::vector<GameRelativePath>&>);
static_assert(std::is_same_v<decltype(std::declval<const FileOwnershipGraph&>().obstructions()),
                             const std::vector<PathObstruction>&>);

namespace {

auto packageGraph(std::vector<PackageId> enabled, std::vector<PackageDependency> dependencies = {},
                  std::vector<PackageOrderConstraint> ordering = {}) -> PackageDependencyGraph {
  return PackageDependencyGraph::create(std::move(enabled), std::move(dependencies),
                                        std::move(ordering))
      .value();
}

auto claim(std::string package, std::string source, std::string destination,
           PackageEntryKind kind = PackageEntryKind::RegularFile) -> PackagePathClaim {
  return PackagePathClaim{PackageId{std::move(package)}, PackageRelativePath{std::move(source)},
                          GameRelativePath{std::move(destination)}, kind};
}

auto requireMissingPackage(std::expected<FileOwnershipGraph, FileOwnershipGraphError> result,
                           const PackageId& package_id, std::size_t claim_index) -> void {
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error().code() == FileOwnershipGraphErrorCode::ClaimPackageNotEnabled);
  REQUIRE(result.error().packageId() == package_id);
  REQUIRE(result.error().claimIndex() == claim_index);
}

} // namespace

TEST_CASE("File ownership graph supports empty and individually mapped claims",
          "[domain][file-ownership]") {
  const auto empty_package_graph = packageGraph({});
  const auto empty = FileOwnershipGraph::create(empty_package_graph, {});
  REQUIRE(empty.has_value());
  REQUIRE(empty->packageOrder().empty());
  REQUIRE(empty->claims().empty());
  REQUIRE(empty->nodes().empty());
  REQUIRE(empty->exactCollisionPaths().empty());
  REQUIRE(empty->obstructions().empty());

  const PackageRootMapping root_mapping(PackageRelativePath{"Root"}, GameRelativePath{""});
  const PackageRootMapping data_mapping(PackageRelativePath{"Data"}, GameRelativePath{"Data"});
  const auto root_destination = root_mapping.map(PackageRelativePath{"Root/loader.so"})
                                    .value_or(GameRelativePath{"mapping-failed"});
  const auto data_destination = data_mapping.map(PackageRelativePath{"Data/textures/a.dds"})
                                    .value_or(GameRelativePath{"mapping-failed"});
  REQUIRE(root_destination == GameRelativePath{"loader.so"});
  REQUIRE(data_destination == GameRelativePath{"Data/textures/a.dds"});

  const auto packages = packageGraph({PackageId{"mapped"}});
  const auto single = FileOwnershipGraph::create(
      packages, {PackagePathClaim{PackageId{"mapped"}, PackageRelativePath{"Root/loader.so"},
                                  root_destination, PackageEntryKind::RegularFile}});
  REQUIRE(single.has_value());
  REQUIRE(single->claims().size() == 1);
  REQUIRE(single->nodes().size() == 1);
  REQUIRE(single->exactCollisionPaths().empty());
  REQUIRE(single->obstructions().empty());

  const std::vector<PackagePathClaim> claims{
      PackagePathClaim{PackageId{"mapped"}, PackageRelativePath{"Root/loader.so"}, root_destination,
                       PackageEntryKind::RegularFile},
      PackagePathClaim{PackageId{"mapped"}, PackageRelativePath{"Data/textures/a.dds"},
                       data_destination, PackageEntryKind::RegularFile},
  };
  const auto graph = FileOwnershipGraph::create(packages, claims);

  REQUIRE(graph.has_value());
  REQUIRE(graph->packageOrder() == std::vector<PackageId>{PackageId{"mapped"}});
  REQUIRE(graph->claims() == claims);
  REQUIRE(graph->nodes().size() == 2);
  REQUIRE(graph->nodes().at(0).path() == GameRelativePath{"Data/textures/a.dds"});
  REQUIRE(graph->nodes().at(1).path() == GameRelativePath{"loader.so"});
  REQUIRE(graph->exactCollisionPaths().empty());
  REQUIRE(graph->obstructions().empty());
}

TEST_CASE("File ownership graph supports conflict-free claims from multiple packages",
          "[domain][file-ownership]") {
  const auto packages = packageGraph({PackageId{"a"}, PackageId{"b"}});
  const auto graph = FileOwnershipGraph::create(
      packages, {claim("b", "b/source", "z/file"), claim("a", "a/source", "a/file")});

  REQUIRE(graph.has_value());
  REQUIRE(graph->nodes() ==
          std::vector<FileOwnershipNode>{
              FileOwnershipNode{GameRelativePath{"a/file"}, {claim("a", "a/source", "a/file")}},
              FileOwnershipNode{GameRelativePath{"z/file"}, {claim("b", "b/source", "z/file")}}});
  REQUIRE(graph->exactCollisionPaths().empty());
  REQUIRE(graph->obstructions().empty());
}

TEST_CASE("File ownership graph orders nodes and claims deterministically",
          "[domain][file-ownership]") {
  const auto packages =
      packageGraph({PackageId{"later"}, PackageId{"earlier"}, PackageId{"independent"}},
                   {PackageDependency{PackageId{"later"}, PackageId{"earlier"}}});
  std::vector<PackagePathClaim> claims{
      claim("later", "late/source", "shared/file"),
      claim("independent", "independent/source", "z/file"),
      claim("earlier", "early/first", "shared/file"),
      claim("earlier", "early/second", "shared/file"),
      claim("later", "late/other", "a/file"),
  };

  const auto first = FileOwnershipGraph::create(packages, claims);
  const auto second = FileOwnershipGraph::create(packages, claims);
  claims.clear();

  REQUIRE(first.has_value());
  REQUIRE(second.has_value());
  REQUIRE(first == second);
  REQUIRE(first->packageOrder() == std::vector<PackageId>{PackageId{"earlier"}, PackageId{"later"},
                                                          PackageId{"independent"}});
  REQUIRE(first->claims().size() == 5);
  REQUIRE(first->nodes().size() == 3);
  REQUIRE(first->nodes().at(0).path() == GameRelativePath{"a/file"});
  REQUIRE(first->nodes().at(1).path() == GameRelativePath{"shared/file"});
  REQUIRE(first->nodes().at(2).path() == GameRelativePath{"z/file"});
  REQUIRE(first->nodes().at(1).claims() ==
          std::vector<PackagePathClaim>{claim("earlier", "early/first", "shared/file"),
                                        claim("earlier", "early/second", "shared/file"),
                                        claim("later", "late/source", "shared/file")});
  REQUIRE(first->exactCollisionPaths() ==
          std::vector<GameRelativePath>{GameRelativePath{"shared/file"}});
}

TEST_CASE("File ownership graph preserves every exact collision claim",
          "[domain][file-ownership]") {
  const auto packages = packageGraph({PackageId{"a"}, PackageId{"b"}});
  const auto repeated = claim("a", "same/source", "same/path");
  const std::vector<PackagePathClaim> claims{
      repeated,
      repeated,
      claim("a", "other/source", "same/path", PackageEntryKind::SymbolicLink),
      claim("b", "package-b/source", "same/path"),
      claim("b", "single/source", "single/path"),
  };
  const auto graph = FileOwnershipGraph::create(packages, claims);

  REQUIRE(graph.has_value());
  REQUIRE(graph->claims() == claims);
  REQUIRE(graph->nodes().at(0).path() == GameRelativePath{"same/path"});
  REQUIRE(graph->nodes().at(0).claims().size() == 4);
  REQUIRE(graph->nodes().at(0).claims().at(0) == repeated);
  REQUIRE(graph->nodes().at(0).claims().at(1) == repeated);
  REQUIRE(graph->exactCollisionPaths() ==
          std::vector<GameRelativePath>{GameRelativePath{"same/path"}});
}

TEST_CASE("File ownership graph merges directories but reports mixed exact kinds",
          "[domain][file-ownership]") {
  const auto packages = packageGraph({PackageId{"a"}, PackageId{"b"}});
  const auto graph = FileOwnershipGraph::create(
      packages, {claim("a", "a/dir", "merged", PackageEntryKind::Directory),
                 claim("b", "b/dir", "merged", PackageEntryKind::Directory),
                 claim("a", "a/mixed", "mixed", PackageEntryKind::Directory),
                 claim("b", "b/mixed", "mixed", PackageEntryKind::RegularFile),
                 claim("a", "a/link", "linked", PackageEntryKind::Directory),
                 claim("b", "b/link", "linked", PackageEntryKind::SymbolicLink)});

  REQUIRE(graph.has_value());
  REQUIRE(graph->nodes().size() == 3);
  REQUIRE(graph->nodes().at(1).path() == GameRelativePath{"merged"});
  REQUIRE(graph->nodes().at(1).claims().size() == 2);
  REQUIRE(graph->exactCollisionPaths() ==
          std::vector<GameRelativePath>{GameRelativePath{"linked"}, GameRelativePath{"mixed"}});
}

TEST_CASE("File ownership graph records file and symlink ancestor obstructions",
          "[domain][file-ownership]") {
  const auto packages = packageGraph({PackageId{"package"}});
  const auto graph = FileOwnershipGraph::create(
      packages, {claim("package", "source/file", "Data/file"),
                 claim("package", "source/child", "Data/file/child"),
                 claim("package", "source/grandchild", "Data/file/child/grandchild"),
                 claim("package", "source/lookalike", "Data/filed"),
                 claim("package", "source/link", "Links/current", PackageEntryKind::SymbolicLink),
                 claim("package", "source/link-child", "Links/current/item"),
                 claim("package", "source/directory", "Merged", PackageEntryKind::Directory),
                 claim("package", "source/directory-child", "Merged/item")});

  REQUIRE(graph.has_value());
  REQUIRE(graph->obstructions() ==
          std::vector<PathObstruction>{
              PathObstruction{GameRelativePath{"Data/file"}, GameRelativePath{"Data/file/child"}},
              PathObstruction{GameRelativePath{"Data/file"},
                              GameRelativePath{"Data/file/child/grandchild"}},
              PathObstruction{GameRelativePath{"Data/file/child"},
                              GameRelativePath{"Data/file/child/grandchild"}},
              PathObstruction{GameRelativePath{"Links/current"},
                              GameRelativePath{"Links/current/item"}}});
}

TEST_CASE("File ownership graph handles empty-root obstructions and arbitrary bytes",
          "[domain][file-ownership]") {
  std::string high_byte_path;
  high_byte_path.push_back(static_cast<char>(0xFF));
  high_byte_path += "/item";
  std::string high_byte_source{"source/"};
  high_byte_source.push_back(static_cast<char>(0xFE));

  const auto packages = packageGraph({PackageId{"package"}});
  const auto graph =
      FileOwnershipGraph::create(packages, {claim("package", "root/file", ""),
                                            claim("package", high_byte_source, high_byte_path),
                                            claim("package", "source/ascii", "z/item")});

  REQUIRE(graph.has_value());
  REQUIRE(graph->nodes().at(0).path() == GameRelativePath{""});
  REQUIRE(graph->nodes().at(1).path() == GameRelativePath{"z/item"});
  REQUIRE(graph->nodes().at(2).path() == GameRelativePath{high_byte_path});
  REQUIRE(graph->nodes().at(2).claims().at(0).packageRelativePath() ==
          PackageRelativePath{high_byte_source});
  REQUIRE(graph->obstructions() ==
          std::vector<PathObstruction>{
              PathObstruction{GameRelativePath{""}, GameRelativePath{"z/item"}},
              PathObstruction{GameRelativePath{""}, GameRelativePath{high_byte_path}}});

  const auto directory_root = FileOwnershipGraph::create(
      packages, {claim("package", "root/directory", "", PackageEntryKind::Directory),
                 claim("package", "source/child", "child")});
  REQUIRE(directory_root.has_value());
  REQUIRE(directory_root->obstructions().empty());
}

TEST_CASE("File ownership graph rejects the first claim outside the package graph",
          "[domain][file-ownership]") {
  const auto packages = packageGraph({PackageId{"enabled"}});

  requireMissingPackage(
      FileOwnershipGraph::create(packages, {claim("first-missing", "source", "path"),
                                            claim("second-missing", "source", "path")}),
      PackageId{"first-missing"}, 0);

  requireMissingPackage(
      FileOwnershipGraph::create(packages, {claim("enabled", "source", "path"),
                                            claim("later-missing", "source", "other")}),
      PackageId{"later-missing"}, 1);
}
