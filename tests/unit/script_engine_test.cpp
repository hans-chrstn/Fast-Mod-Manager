#include "infrastructure/StubScriptEngine.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("StubScriptEngine evaluates mock scripts", "[infrastructure][StubScriptEngine]") {
  fmm::infrastructure::StubScriptEngine engine;

  SECTION("Successfully evaluates valid script string") {
    auto result = engine.evaluate("return 42");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "Evaluated: return 42");
  }

  SECTION("Fails on empty script") {
    auto result = engine.evaluate("");
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "Empty script");
  }
}
