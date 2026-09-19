#pragma once

#include <QMainWindow>

class QListView;
class QLineEdit;
class QSortFilterProxyModel;
class QProgressBar;
class QLabel;
class QPushButton;

namespace ui {

class InventoryModel;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(InventoryModel* inventory_model, QWidget* parent = nullptr);
  ~MainWindow() override = default;

private:
  void setupUi();

  InventoryModel* m_inventory_model;
  QSortFilterProxyModel* m_proxy_model{nullptr};
  QLineEdit* m_search_bar{nullptr};
  QListView* m_list_view{nullptr};
  QProgressBar* m_progress_bar{nullptr};
  QLabel* m_status_label{nullptr};
  QPushButton* m_cancel_button{nullptr};
  QWidget* m_progress_widget{nullptr};

  Q_SLOT void onScanProgress(int percentage, const QString& message);
  Q_SLOT void onScanCompleted();
  Q_SLOT void onScanFailed(const QString& error);
  Q_SLOT void onCancelClicked();
};

} // namespace ui
