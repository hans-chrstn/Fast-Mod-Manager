#pragma once

#include "core/ServiceRegistry.hpp"

#include <QApplication>
#include <memory>

namespace application {
class InventoryService;
}

namespace ui {
class InventoryModel;
class MainWindow;
} // namespace ui

namespace app {

class Bootstrapper {
public:
  Bootstrapper(int& argc, char** argv);
  ~Bootstrapper();

  auto run() -> int;

private:
  void buildServiceGraph();

  QApplication m_application;
  core::ServiceRegistry m_registry;

  std::unique_ptr<ui::InventoryModel> m_inventory_model;
  std::unique_ptr<ui::MainWindow> m_main_window;
};

} // namespace app
