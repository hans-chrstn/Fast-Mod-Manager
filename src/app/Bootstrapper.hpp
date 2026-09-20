#pragma once

#include "core/ServiceRegistry.hpp"

#include <QApplication>
#include <memory>

namespace fmm::application {
class InventoryService;
}

namespace fmm::ui {
class InventoryModel;
class MainWindow;
} // namespace fmm::ui

namespace fmm::app {

class Bootstrapper {
public:
  Bootstrapper(int& argc, char** argv);
  ~Bootstrapper();

  auto run() -> int;

private:
  void buildServiceGraph();

  QApplication m_application;
  fmm::core::ServiceRegistry m_registry;

  std::unique_ptr<fmm::ui::InventoryModel> m_inventory_model;
  std::unique_ptr<fmm::ui::MainWindow> m_main_window;
};

} // namespace fmm::app
