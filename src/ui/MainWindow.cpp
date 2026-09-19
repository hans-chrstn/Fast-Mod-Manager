#include "ui/MainWindow.hpp"

#include "ui/InventoryModel.hpp"

#include <QAction>
#include <QApplication>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QMenuBar>
#include <QProgressBar>
#include <QPushButton>
#include <QSortFilterProxyModel>
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

  m_search_bar = new QLineEdit(central_widget);
  m_search_bar->setPlaceholderText(QStringLiteral("Filter mods..."));
  layout->addWidget(m_search_bar);

  m_proxy_model = new QSortFilterProxyModel(this);
  m_proxy_model->setSourceModel(m_inventory_model);
  m_proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);

  connect(m_search_bar, &QLineEdit::textChanged, m_proxy_model,
          &QSortFilterProxyModel::setFilterFixedString);

  m_list_view = new QListView(central_widget);
  m_list_view->setModel(m_proxy_model);
  layout->addWidget(m_list_view);

  m_progress_widget = new QWidget(central_widget);
  auto* progress_layout = new QHBoxLayout(m_progress_widget);
  progress_layout->setContentsMargins(0, 0, 0, 0);

  m_status_label = new QLabel(QStringLiteral("Ready"), m_progress_widget);
  progress_layout->addWidget(m_status_label);

  m_progress_bar = new QProgressBar(m_progress_widget);
  constexpr int max_progress = 100;
  m_progress_bar->setRange(0, max_progress);
  m_progress_bar->setValue(0);
  progress_layout->addWidget(m_progress_bar);

  m_cancel_button = new QPushButton(QStringLiteral("Cancel"), m_progress_widget);
  connect(m_cancel_button, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
  progress_layout->addWidget(m_cancel_button);

  layout->addWidget(m_progress_widget);
  m_progress_widget->hide();

  setCentralWidget(central_widget);
}

void MainWindow::onScanProgress(int percentage, const QString& message) {
  if (m_progress_widget->isHidden()) {
    m_progress_widget->show();
  }
  m_progress_bar->setValue(percentage);
  m_status_label->setText(message);
}

void MainWindow::onScanCompleted() {
  m_progress_widget->hide();
  m_status_label->setText(QStringLiteral("Ready"));
}

void MainWindow::onScanFailed(const QString& error) {
  m_progress_widget->hide();
  m_status_label->setText(QStringLiteral("Error: ") + error);
}

void MainWindow::onCancelClicked() {
  m_inventory_model->cancelReload();
  m_progress_widget->hide();
  m_status_label->setText(QStringLiteral("Cancelled"));
}

} // namespace ui
