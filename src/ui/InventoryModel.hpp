#pragma once

#include "domain/ModIdentity.hpp"

#include <QAbstractListModel>
#include <QObject>
#include <memory>
#include <vector>

namespace application {
class InventoryService;
}

namespace ui {

class InventoryModel : public QAbstractListModel {
  Q_OBJECT

public:
  explicit InventoryModel(std::shared_ptr<application::InventoryService> inventory_service,
                          QObject* parent = nullptr);
  ~InventoryModel() override;

  [[nodiscard]] auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;
  [[nodiscard]] auto data(const QModelIndex& index, int role = Qt::DisplayRole) const
      -> QVariant override;

  void reload();

private:
  std::shared_ptr<application::InventoryService> m_inventory_service;
  std::vector<domain::ModIdentity> m_mods;
};

} // namespace ui
