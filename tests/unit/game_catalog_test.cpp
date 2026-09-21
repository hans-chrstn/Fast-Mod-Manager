#include "application/GameCatalogImpl.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace fmm::application;
using namespace fmm::domain;

namespace {
auto makeDefinition(std::string id, std::string name) -> GameDefinition {
  return GameDefinition{
      .identity = GameIdentity(GameId{std::move(id)}, std::move(name)),
      .capabilities = {},
      .executable_name = "game.exe",
      .mod_directory_name = "Data",
      .required_tools = {},
  };
}
} // namespace

TEST_CASE("GameCatalog resolves definitions by stable GameId", "[application][game-catalog]") {
  GameCatalogImpl catalog;
  catalog.registerGame(makeDefinition("game-a", "Shared Display Name"));
  catalog.registerGame(makeDefinition("game-b", "Shared Display Name"));

  const auto game_b = catalog.getGameDefinition(GameId{"game-b"});
  REQUIRE(game_b.has_value());
  REQUIRE(game_b.transform([](const GameDefinition& definition) {
    return definition.identity.id();
  }) == GameId{"game-b"});
  REQUIRE(game_b.transform([](const GameDefinition& definition) {
    return definition.identity.name();
  }) == "Shared Display Name");

  REQUIRE_FALSE(catalog.getGameDefinition(GameId{"missing-game"}).has_value());
}
