#include "infrastructure/LuaGameAdapter.hpp"
#include "infrastructure/StubScriptEngine.hpp"

#include <catch2/catch_test_macros.hpp>
#include <memory>

TEST_CASE("LuaGameAdapter yields a GameDefinition", "[infrastructure][LuaGameAdapter]") {
  auto script_engine = std::make_shared<fmm::infrastructure::StubScriptEngine>();
  fmm::infrastructure::LuaGameAdapter adapter(script_engine, "/fake/path/to/mygame/init.lua");

  SECTION("getGameDefinition returns deterministic defaults") {
    auto def = adapter.getGameDefinition();

    REQUIRE(def.game_id == "mygame");
    REQUIRE(def.executable_name == "mygame.exe");
    REQUIRE(def.mod_directory_name == "Mods");
    REQUIRE(def.name == "Unknown Game");
  }
}
