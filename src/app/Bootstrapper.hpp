#pragma once

#include "core/ServiceRegistry.hpp"

#include <QApplication>
#include <functional>
#include <memory>

namespace fmm::application {
class InventoryService;
}

namespace fmm::ui {
class InventoryModel;
class MainWindow;
} // namespace fmm::ui

namespace fmm::app {

using CompositionCallback = std::function<void(fmm::core::ServiceRegistry&)>;

class Bootstrapper {
public:
  Bootstrapper(int& argc, char** argv, const CompositionCallback& composition_hook = nullptr);
  ~Bootstrapper();

  auto run() -> int;

private:
  void buildServiceGraph(const CompositionCallback& composition_hook);

  QApplication m_application;
  fmm::core::ServiceRegistry m_registry;

  std::unique_ptr<fmm::ui::InventoryModel> m_inventory_model;
  std::unique_ptr<fmm::ui::MainWindow> m_main_window;
};

} // namespace fmm::app
