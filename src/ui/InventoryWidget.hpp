#pragma once

#include <QWidget>

class QListView;
class QLineEdit;
class QSortFilterProxyModel;

namespace ui {

class InventoryModel;

class InventoryWidget : public QWidget {
  Q_OBJECT

public:
  explicit InventoryWidget(InventoryModel* inventory_model, QWidget* parent = nullptr);
  ~InventoryWidget() override = default;

private:
  void setupUi();

  InventoryModel* m_inventory_model;
  QSortFilterProxyModel* m_proxy_model{nullptr};
  QLineEdit* m_search_bar{nullptr};
  QListView* m_list_view{nullptr};
};

} // namespace ui
