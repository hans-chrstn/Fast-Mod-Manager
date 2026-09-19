#include "app/Bootstrapper.hpp"

#include "application/FakeInventoryService.hpp"
#include "application/InventoryService.hpp"
#include "core/IDependencyValidator.hpp"
#include "core/IModScanner.hpp"
#include "core/IProcessLauncher.hpp"
#include "core/IScriptEngine.hpp"
#include "infrastructure/DependencyValidatorImpl.hpp"
#include "infrastructure/FixtureFilesystemScanner.hpp"
#include "infrastructure/StubProcessLauncher.hpp"
#include "infrastructure/StubScriptEngine.hpp"
#include "ui/InventoryModel.hpp"
#include "ui/MainWindow.hpp"

#include <QMessageBox>
#include <memory>
#include <numeric>

namespace app {

Bootstrapper::Bootstrapper(int& argc, char** argv) : m_application(argc, argv) {
  buildServiceGraph();
  m_main_window->show();
}

Bootstrapper::~Bootstrapper() = default;

void Bootstrapper::buildServiceGraph() {
  m_registry.registerService<fmm::core::IDependencyValidator>(
      std::make_shared<fmm::infrastructure::DependencyValidatorImpl>());

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
  auto validator = m_registry.resolve<fmm::core::IDependencyValidator>();

  auto check_dependencies = [this, validator]() -> void {
    auto missing = validator->getMissingDependencies();
    if (!missing.empty()) {
      QString msg =
          "The following prerequisite packages or requirements are missing or outdated:\n\n";
      msg = std::accumulate(missing.begin(), missing.end(), msg,
                            [](const QString& acc, const auto& issue) -> QString {
                              return acc + QString::fromStdString("- " + issue.name + ": " +
                                                                  issue.description + "\n  " +
                                                                  issue.resolution_hint + "\n\n");
                            });
      QMessageBox::warning(m_main_window.get(), "Missing Prerequisites", msg);
    } else {
      QMessageBox::information(m_main_window.get(), "Prerequisites Checked",
                               "All prerequisite packages and requirements are satisfied.");
    }
  };

  QObject::connect(m_main_window.get(), &ui::MainWindow::requestDependencyCheck,
                   check_dependencies);

  auto initial_missing = validator->getMissingDependencies();
  if (!initial_missing.empty()) {
    check_dependencies();
  }

  m_main_window->show();
  return QApplication::exec();
}

} // namespace app
