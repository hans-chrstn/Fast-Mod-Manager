#include "ui/MainWindow.hpp"

#include "ui/InventoryModel.hpp"

#include <QAction>
#include <QApplication>
#include <QKeySequence>
#include <QListView>
#include <QMenu>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QWidget>

namespace ui {

MainWindow::MainWindow(InventoryModel* inventory_model, QWidget* parent)
    : QMainWindow(parent), m_inventory_model(inventory_model) {
  setupUi();
}

void MainWindow::setupUi() {
  setWindowTitle(QStringLiteral("Fast Mod Manager"));
  constexpr int window_width = 960;
  constexpr int window_height = 640;
  resize(window_width, window_height);

  auto* file_menu = menuBar()->addMenu(QStringLiteral("&File"));
  auto* quit_action = file_menu->addAction(QStringLiteral("&Quit"));
  quit_action->setShortcut(QKeySequence::Quit);
  connect(quit_action, &QAction::triggered, qApp, &QApplication::quit);

  auto* central_widget = new QWidget(this);
  auto* layout = new QVBoxLayout(central_widget);

  m_list_view = new QListView(central_widget);
  m_list_view->setModel(m_inventory_model);
  layout->addWidget(m_list_view);

  setCentralWidget(central_widget);
}

} // namespace ui
