#include "domain/PlannerInput.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <type_traits>
#include <utility>

using namespace fmm::domain;

namespace {
auto makeGameDefinition() -> GameDefinition {
  return GameDefinition{
      .identity = GameIdentity("test-game", "Test Game"),
      .capabilities = GameCapabilities{.filesystem_semantics = "windows",
                                       .required_features = {"load_order"},
                                       .preferred_features = {"reflink"}},
      .executable_name = "test-game.exe",
      .mod_directory_name = "Data",
      .required_tools = {"script-extender"},
  };
}

auto makeInstalledPackage(std::string package_id, std::string name,
                          std::filesystem::path staging_path) -> InstalledPackage {
  return InstalledPackage(
      PackageId{std::move(package_id)},
      PackageMetadata{.name = std::move(name), .version = "1.0", .source = "test"},
      PackageLocation{.staging_path = std::move(staging_path)});
}
} // namespace

static_assert(std::is_same_v<decltype(std::declval<const PlannerInput&>().profileState()),
                             const ProfileState&>);
static_assert(std::is_same_v<decltype(std::declval<const PlannerInput&>().gameDefinition()),
                             const GameDefinition&>);
static_assert(std::is_same_v<decltype(std::declval<const PlannerInput&>().installedPackages()),
                             const std::vector<InstalledPackage>&>);

TEST_CASE("PlannerInput composes profile, game, and installed package state", "[domain][planner]") {
  ProfileState profile{
      .selected_game_id = "test-game",
      .enabled_packages = {PackageId{"package-b"}, PackageId{"package-a"}},
      .load_order = {"plugin-b", "plugin-a"},
  };
  const auto game = makeGameDefinition();
  std::vector<InstalledPackage> packages;
  packages.push_back(makeInstalledPackage("package-a", "Package A", "/managed/package-a"));
  packages.push_back(makeInstalledPackage("package-b", "Package B", "/managed/package-b"));

  const PlannerInput input(profile, game, packages);

  REQUIRE(input.profileState().selected_game_id == "test-game");
  REQUIRE(input.profileState().enabled_packages == profile.enabled_packages);
  REQUIRE(input.profileState().load_order == profile.load_order);
  REQUIRE(input.gameDefinition().identity == game.identity);
  REQUIRE(input.gameDefinition().capabilities == game.capabilities);
  REQUIRE(input.gameDefinition().executable_name == game.executable_name);
  REQUIRE(input.installedPackages() == packages);
}

TEST_CASE("PlannerInput owns a stable snapshot of its source values", "[domain][planner]") {
  ProfileState profile{.selected_game_id = "test-game"};
  auto game = makeGameDefinition();
  std::vector<InstalledPackage> packages;
  packages.push_back(makeInstalledPackage("package-a", "Package A", "/managed/package-a"));
  const PlannerInput input(profile, game, packages);

  profile.selected_game_id = "changed-game";
  game.executable_name = "changed.exe";
  packages.clear();

  REQUIRE(input.profileState().selected_game_id == "test-game");
  REQUIRE(input.gameDefinition().executable_name == "test-game.exe");
  REQUIRE(input.installedPackages().size() == 1);
  REQUIRE(input.installedPackages().front().id() == PackageId{"package-a"});
}

TEST_CASE("PlannerInput supports a fresh profile with no installed packages", "[domain][planner]") {
  const PlannerInput input(ProfileState{.selected_game_id = "test-game"}, makeGameDefinition(),
                           std::vector<InstalledPackage>{});

  REQUIRE(input.installedPackages().empty());
}
