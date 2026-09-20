#include "support/StubProcessLauncher.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("StubProcessLauncher execution", "[test-support][StubProcessLauncher]") {
  fmm::test_support::StubProcessLauncher launcher;

  SECTION("Successfully launches valid executable") {
    auto result = launcher.launch("/usr/bin/env", {}, "");
    REQUIRE(result.has_value());
  }

  SECTION("Fails on empty executable path") {
    auto result = launcher.launch("", {}, "");
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == fmm::core::ProcessLauncherError::ExecutableNotFound);
  }

  SECTION("Fails on simulated failure") {
    auto result = launcher.launch("/bin/false", {}, "/tmp");
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == fmm::core::ProcessLauncherError::ExecutionFailed);
  }
}
