#include "app/Bootstrapper.hpp"
#include "application/FakeInventoryService.hpp"
#include "core/IModScanner.hpp"
#include "core/IProcessLauncher.hpp"
#include "core/IScriptEngine.hpp"
#include "infrastructure/FixtureFilesystemScanner.hpp"
#include "infrastructure/StubProcessLauncher.hpp"
#include "infrastructure/StubScriptEngine.hpp"

#include <exception>
#include <iostream>
#include <memory>

auto main(int argc, char* argv[]) -> int {
  try {
    fmm::app::Bootstrapper bootstrapper(
        argc, argv, [](fmm::core::ServiceRegistry& registry) -> void {
          registry.registerService<fmm::core::IModScanner>(
              std::make_shared<fmm::infrastructure::FixtureFilesystemScanner>());

          registry.registerService<fmm::core::IProcessLauncher>(
              std::make_shared<fmm::infrastructure::StubProcessLauncher>());

          registry.registerService<fmm::core::IScriptEngine>(
              std::make_shared<fmm::infrastructure::StubScriptEngine>());

          registry.registerService<fmm::application::InventoryService>(
              std::make_shared<fmm::application::FakeInventoryService>(
                  registry.resolve<fmm::core::IModScanner>()));
        });
    return bootstrapper.run();
  } catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << '\n';
    return 1;
  } catch (...) {
    std::cerr << "Fatal unknown error\n";
    return 1;
  }
}
