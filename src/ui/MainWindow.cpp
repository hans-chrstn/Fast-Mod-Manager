#include "ui/MainWindow.hpp"

#include "ui/InventoryModel.hpp"
#include "ui/InventoryWidget.hpp"
#include "ui/ProgressOverlayWidget.hpp"

#include <QAction>
#include <QApplication>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>

namespace ui {

MainWindow::MainWindow(InventoryModel* inventory_model, QWidget* parent)
    : QMainWindow(parent), m_inventory_model(inventory_model) {
  setupUi();

  connect(m_inventory_model, &InventoryModel::scanProgress, this, &MainWindow::onScanProgress);
  connect(m_inventory_model, &InventoryModel::scanCompleted, this, &MainWindow::onScanCompleted);
  connect(m_inventory_model, &InventoryModel::scanFailed, this, &MainWindow::onScanFailed);

  m_inventory_model->reload();
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

  m_inventory_widget = new InventoryWidget(m_inventory_model, central_widget);
  layout->addWidget(m_inventory_widget);

  m_progress_overlay = new ProgressOverlayWidget(central_widget);
  connect(m_progress_overlay, &ProgressOverlayWidget::cancelRequested, this,
          &MainWindow::onCancelClicked);
  layout->addWidget(m_progress_overlay);
  m_progress_overlay->hideOverlay();

  setCentralWidget(central_widget);
}

void MainWindow::onScanProgress(int percentage, const QString& message) {
  if (m_progress_overlay->isHidden()) {
    m_progress_overlay->showOverlay();
  }
  m_progress_overlay->setProgress(percentage, message);
}

void MainWindow::onScanCompleted() {
  m_progress_overlay->hideOverlay();
  m_progress_overlay->resetState();
}

void MainWindow::onScanFailed(const QString& error) {
  m_progress_overlay->hideOverlay();
  m_progress_overlay->resetState();
  QMessageBox::critical(this, QStringLiteral("Scan Failed"), error);
}

void MainWindow::onCancelClicked() {
  m_inventory_model->cancelReload();
  m_progress_overlay->hideOverlay();
  m_progress_overlay->resetState();
}

} // namespace ui
