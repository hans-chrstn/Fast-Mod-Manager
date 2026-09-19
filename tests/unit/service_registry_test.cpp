#include "core/ServiceRegistry.hpp"

#include <catch2/catch_test_macros.hpp>

namespace {

class ITestService {
public:
  virtual ~ITestService() = default;
  [[nodiscard]] virtual auto getValue() const -> int = 0;
};

class TestServiceImpl : public ITestService {
public:
  [[nodiscard]] auto getValue() const -> int override {
    constexpr int expected_value = 42;
    return expected_value;
  }
};

class IOtherService {
public:
  virtual ~IOtherService() = default;
};

} // namespace

TEST_CASE("ServiceRegistry manages service lifecycles", "[core][ServiceRegistry]") {
  core::ServiceRegistry registry;

  SECTION("Can register and resolve a service") {
    auto service = std::make_shared<TestServiceImpl>();
    registry.registerService<ITestService>(service);

    REQUIRE(registry.hasService<ITestService>());

    auto resolved = registry.resolve<ITestService>();
    REQUIRE(resolved != nullptr);
    REQUIRE(resolved->getValue() == 42);
  }

  SECTION("Throws on resolving unknown service") {
    REQUIRE_FALSE(registry.hasService<IOtherService>());
    REQUIRE_THROWS_AS(registry.resolve<IOtherService>(), std::runtime_error);
  }

  SECTION("Throws on null service registration") {
    std::shared_ptr<ITestService> null_service;
    REQUIRE_THROWS_AS(registry.registerService<ITestService>(null_service), std::invalid_argument);
  }
}
