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
}
