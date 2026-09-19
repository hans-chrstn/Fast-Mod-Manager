#pragma once

#include <QMainWindow>

class QListView;

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
  QListView* m_list_view{nullptr};
};

} // namespace ui
