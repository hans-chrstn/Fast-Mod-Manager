#include "infrastructure/StubScriptEngine.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("StubScriptEngine evaluates mock scripts", "[infrastructure][StubScriptEngine]") {
  fmm::infrastructure::StubScriptEngine engine;

  SECTION("Successfully evaluates valid script string") {
    auto result = engine.evaluateGamePlugin("return 42");
    REQUIRE(result.has_value());
    REQUIRE(result.value().identity.id() == "stub_game");
    REQUIRE(result.value().capabilities.hasRequiredFeature("plugins") == true);
  }

  SECTION("Fails on empty script") {
    auto result = engine.evaluateGamePlugin("");
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code == fmm::core::GamePluginErrorCode::InvalidFormat);
  }
}
