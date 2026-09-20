#include "app/Bootstrapper.hpp"

#include "application/InventoryService.hpp"
#include "core/IDependencyValidator.hpp"
#include "infrastructure/DependencyValidatorImpl.hpp"
#include "ui/InventoryModel.hpp"
#include "ui/MainWindow.hpp"

#include <QMessageBox>
#include <memory>
#include <numeric>

namespace fmm::app {

Bootstrapper::Bootstrapper(int& argc, char** argv, const CompositionCallback& composition_hook)
    : m_application(argc, argv) {
  buildServiceGraph(composition_hook);
  m_main_window->show();
}

Bootstrapper::~Bootstrapper() = default;

void Bootstrapper::buildServiceGraph(const CompositionCallback& composition_hook) {
  m_registry.registerService<fmm::core::IDependencyValidator>(
      std::make_shared<fmm::infrastructure::DependencyValidatorImpl>());

  if (composition_hook) {
    composition_hook(m_registry);
  }

  m_inventory_model = std::make_unique<fmm::ui::InventoryModel>(
      m_registry.resolve<fmm::application::InventoryService>(), nullptr);
  m_main_window = std::make_unique<fmm::ui::MainWindow>(m_inventory_model.get());
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

  QObject::connect(m_main_window.get(), &fmm::ui::MainWindow::requestDependencyCheck,
                   check_dependencies);

  auto initial_missing = validator->getMissingDependencies();
  if (!initial_missing.empty()) {
    check_dependencies();
  }

  m_main_window->show();
  return QApplication::exec();
}

} // namespace fmm::app
