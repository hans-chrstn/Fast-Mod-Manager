#include "domain/GameIdentity.hpp"
#include "infrastructure/GameManagerImpl.hpp"
#include "infrastructure/StubScriptEngine.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

TEST_CASE("GameManagerImpl dynamic discovery", "[infrastructure][GameManagerImpl]") {
  auto script_engine = std::make_shared<fmm::infrastructure::StubScriptEngine>();

  std::filesystem::path temp_dir = std::filesystem::temp_directory_path() / "fmm_test_plugins";
  std::filesystem::create_directories(temp_dir / "fallout4");
  std::filesystem::create_directories(temp_dir / "skyrim");
  std::filesystem::create_directories(temp_dir / "empty_dir");

  std::ofstream(temp_dir / "fallout4" / "init.lua") << "return {}";
  std::ofstream(temp_dir / "skyrim" / "init.lua") << "return {}";

  SECTION("Discovers only directories with init.lua") {
    fmm::infrastructure::GameManagerImpl manager(script_engine, temp_dir.string());
    auto games = manager.getAvailableGames();

    REQUIRE(games.size() == 2);

    bool found_fo4 = false;
    bool found_skyrim = false;

    for (const auto& g : games) {
      if (g.id() == "fallout4") {
        found_fo4 = true;
      }
      if (g.id() == "skyrim") {
        found_skyrim = true;
      }
    }

    REQUIRE(found_fo4);
    REQUIRE(found_skyrim);
  }

  SECTION("Can set active game") {
    fmm::infrastructure::GameManagerImpl manager(script_engine, temp_dir.string());

    REQUIRE(!manager.getActiveAdapter().has_value());

    manager.setActiveGame(fmm::domain::GameIdentity("fallout4", "fallout4"));

    auto active = manager.getActiveAdapter();
    REQUIRE(active.has_value());
    REQUIRE(active.value()->getIdentity().id() == "fallout4");
  }

  std::filesystem::remove_all(temp_dir);
}
