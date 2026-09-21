#include "domain/GameId.hpp"
#include "domain/GameIdentity.hpp"
#include "domain/PackageId.hpp"
#include "domain/PluginId.hpp"
#include "domain/ProfileState.hpp"

#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace fmm::domain;

static_assert(std::is_constructible_v<GameId, std::string>);
static_assert(std::is_constructible_v<PluginId, std::string>);
static_assert(!std::is_convertible_v<std::string, GameId>);
static_assert(!std::is_convertible_v<std::string, PluginId>);
static_assert(!std::is_constructible_v<GameId, PluginId>);
static_assert(!std::is_constructible_v<PluginId, PackageId>);
static_assert(std::is_same_v<decltype(std::declval<const GameIdentity&>().id()), const GameId&>);
static_assert(std::is_same_v<decltype(ProfileState::selected_game_id), std::optional<GameId>>);
static_assert(std::is_same_v<decltype(ProfileState::enabled_packages), std::vector<PackageId>>);
static_assert(std::is_same_v<decltype(ProfileState::load_order), std::vector<PluginId>>);

TEST_CASE("Typed planner identities preserve value semantics", "[domain][identity]") {
  const GameId game_a{"game-a"};
  const GameId game_a_copy{"game-a"};
  const GameId game_b{"game-b"};
  const PluginId plugin_a{"plugin-a"};
  const PluginId plugin_b{"plugin-b"};

  REQUIRE(game_a == game_a_copy);
  REQUIRE(game_a < game_b);
  REQUIRE(game_a.value() == "game-a");
  REQUIRE(plugin_a < plugin_b);
  REQUIRE(plugin_a.value() == "plugin-a");
}

TEST_CASE("GameIdentity separates stable ID from display name", "[domain][identity]") {
  const GameIdentity identity(GameId{"stable-game"}, "Display Name");

  REQUIRE(identity.id() == GameId{"stable-game"});
  REQUIRE(identity.name() == "Display Name");
}
