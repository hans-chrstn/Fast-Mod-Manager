#include "infrastructure/GameFsClient.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace fmm::infrastructure;

TEST_CASE("Rust C ABI Boundary: GameFsClient lifecycle", "[rust][abi]") {
  SECTION("Can create and destroy GameFsClient without crashing") {
    GameFsClient client;
    SUCCEED("GameFsClient created and destroyed successfully");
  }

  SECTION("Move semantics work correctly") {
    GameFsClient client1;
    GameFsClient client2(std::move(client1));

    GameFsClient client3;
    client3 = std::move(client2);

    SUCCEED("Move semantics did not trigger double-free or crash");
  }

  SECTION("Can pass the legacy plan scaffold across the C ABI") {
    GameFsClient client;
    fmm::core::LegacyGameFsAbiPlan plan;
    plan.profile_id = "test_profile";
    plan.target_game_directory = "/tmp/fake_game";
    plan.deployed_files = {"/tmp/fake_game/mod.dll", "/tmp/fake_game/Data/textures.pak"};

    bool result = client.applyPlan(plan);
    REQUIRE(result == true);
  }

  SECTION("Empty plan is handled safely") {
    GameFsClient client;
    fmm::core::LegacyGameFsAbiPlan plan;
    plan.profile_id = "empty";
    plan.target_game_directory = "/tmp/empty";

    bool result = client.applyPlan(plan);
    REQUIRE(result == true);
  }
}
