#include "domain/PackagePlan.hpp"
#include "domain/PackageRootMapping.hpp"

#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

using namespace fmm::domain;

static_assert(std::is_constructible_v<GameRelativePath, std::string>);
static_assert(!std::is_convertible_v<std::string, GameRelativePath>);
static_assert(!std::is_same_v<PackageRelativePath, GameRelativePath>);
static_assert(!std::is_constructible_v<PackageRootMapping, GameRelativePath, PackageRelativePath>);
static_assert(std::is_same_v<decltype(std::declval<const PackageRootMapping&>().packageRoot()),
                             const PackageRelativePath&>);
static_assert(std::is_same_v<decltype(std::declval<const PackageRootMapping&>().gameRoot()),
                             const GameRelativePath&>);
static_assert(std::is_same_v<decltype(std::declval<const PackageRootMapping&>().map(
                                 std::declval<const PackageRelativePath&>())),
                             std::optional<GameRelativePath>>);

TEST_CASE("Package root mappings translate conventional roots as data",
          "[domain][package-root-mapping]") {
  const PackageRootMapping root_mapping(PackageRelativePath{"Root"}, GameRelativePath{""});
  const PackageRootMapping data_mapping(PackageRelativePath{"Data"}, GameRelativePath{"Data"});

  REQUIRE(root_mapping.packageRoot() == PackageRelativePath{"Root"});
  REQUIRE(root_mapping.gameRoot() == GameRelativePath{""});

  const auto exact_root = root_mapping.map(PackageRelativePath{"Root"});
  REQUIRE(exact_root == std::optional<GameRelativePath>{GameRelativePath{""}});

  const auto root_file = root_mapping.map(PackageRelativePath{"Root/loader.so"});
  REQUIRE(root_file == std::optional<GameRelativePath>{GameRelativePath{"loader.so"}});

  const auto data_file = data_mapping.map(PackageRelativePath{"Data/textures/a.dds"});
  REQUIRE(data_file == std::optional<GameRelativePath>{GameRelativePath{"Data/textures/a.dds"}});
}

TEST_CASE("Package root mappings require complete source path segments",
          "[domain][package-root-mapping]") {
  const PackageRootMapping root_mapping(PackageRelativePath{"Root"}, GameRelativePath{""});

  REQUIRE_FALSE(root_mapping.map(PackageRelativePath{"Rooted/file"}).has_value());
  REQUIRE_FALSE(root_mapping.map(PackageRelativePath{"Roo"}).has_value());
  REQUIRE_FALSE(root_mapping.map(PackageRelativePath{"Data/file"}).has_value());

  const PackageRootMapping empty_source_mapping(PackageRelativePath{""}, GameRelativePath{"Data"});
  REQUIRE_FALSE(empty_source_mapping.map(PackageRelativePath{"Data/file"}).has_value());

  const PackageRootMapping custom_mapping(PackageRelativePath{"Content"},
                                          GameRelativePath{"Assets"});
  const auto custom_file = custom_mapping.map(PackageRelativePath{"Content/scripts/init.lua"});
  REQUIRE(custom_file ==
          std::optional<GameRelativePath>{GameRelativePath{"Assets/scripts/init.lua"}});
}

TEST_CASE("Package root mappings preserve entry kinds and arbitrary bytes",
          "[domain][package-root-mapping]") {
  std::string source_bytes{"Data/textures/"};
  source_bytes.push_back(static_cast<char>(0xFF));
  source_bytes += ".dds";

  std::string expected_bytes{"Data/textures/"};
  expected_bytes.push_back(static_cast<char>(0xFF));
  expected_bytes += ".dds";

  const PackageRootMapping data_mapping(PackageRelativePath{"Data"}, GameRelativePath{"Data"});
  const PackageLayoutEntry directory(PackageRelativePath{"Data/textures"},
                                     PackageEntryKind::Directory);
  const PackageLayoutEntry symbolic_link(PackageRelativePath{source_bytes},
                                         PackageEntryKind::SymbolicLink);

  const auto directory_path = data_mapping.map(directory.relativePath());
  const auto symbolic_link_path = data_mapping.map(symbolic_link.relativePath());

  REQUIRE(directory_path == std::optional<GameRelativePath>{GameRelativePath{"Data/textures"}});
  REQUIRE(directory.kind() == PackageEntryKind::Directory);
  REQUIRE(symbolic_link_path == std::optional<GameRelativePath>{GameRelativePath{expected_bytes}});
  REQUIRE(symbolic_link.kind() == PackageEntryKind::SymbolicLink);
}
