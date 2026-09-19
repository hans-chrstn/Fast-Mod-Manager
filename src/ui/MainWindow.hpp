#pragma once

#include <QMainWindow>

class QListView;
class QLineEdit;
class QSortFilterProxyModel;

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
};

} // namespace ui
