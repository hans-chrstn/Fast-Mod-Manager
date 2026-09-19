#include "infrastructure/StubProcessLauncher.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("StubProcessLauncher execution", "[infrastructure][IProcessLauncher]") {
  infrastructure::StubProcessLauncher launcher;

  SECTION("Successfully launches valid executable") {
    auto result = launcher.launch("/bin/true", {}, "/tmp");
    REQUIRE(result.has_value());
  }

  SECTION("Fails on empty executable path") {
    auto result = launcher.launch("", {}, "/tmp");
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == core::ProcessLauncherError::ExecutableNotFound);
  }

  SECTION("Fails on simulated failure") {
    auto result = launcher.launch("/bin/false", {}, "/tmp");
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == core::ProcessLauncherError::ExecutionFailed);
  }
}
