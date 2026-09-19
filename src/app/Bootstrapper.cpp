#include "app/Bootstrapper.hpp"

#include "application/FakeInventoryService.hpp"
#include "application/InventoryService.hpp"
#include "core/IModScanner.hpp"
#include "core/IProcessLauncher.hpp"
#include "core/IScriptEngine.hpp"
#include "infrastructure/FixtureFilesystemScanner.hpp"
#include "infrastructure/StubProcessLauncher.hpp"
#include "infrastructure/StubScriptEngine.hpp"
#include "ui/InventoryModel.hpp"
#include "ui/MainWindow.hpp"

#include <memory>

namespace app {

Bootstrapper::Bootstrapper(int& argc, char** argv) : m_application(argc, argv) {
  buildServiceGraph();
  m_main_window->show();
}

Bootstrapper::~Bootstrapper() = default;

void Bootstrapper::buildServiceGraph() {
  m_registry.registerService<fmm::core::IModScanner>(
      std::make_shared<fmm::infrastructure::FixtureFilesystemScanner>());

  m_registry.registerService<fmm::core::IProcessLauncher>(
      std::make_shared<fmm::infrastructure::StubProcessLauncher>());

  m_registry.registerService<fmm::core::IScriptEngine>(
      std::make_shared<fmm::infrastructure::StubScriptEngine>());

  m_registry.registerService<application::InventoryService>(
      std::make_shared<application::FakeInventoryService>(
          m_registry.resolve<fmm::core::IModScanner>()));

  m_inventory_model = std::make_unique<ui::InventoryModel>(
      m_registry.resolve<application::InventoryService>(), nullptr);
  m_main_window = std::make_unique<ui::MainWindow>(m_inventory_model.get());
}

auto Bootstrapper::run() -> int {
  m_main_window->show();
  return QApplication::exec();
}

} // namespace app
