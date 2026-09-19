#include "ui/InventoryModel.hpp"

#include "application/InventoryService.hpp"

#include <stdexcept>

namespace ui {

InventoryModel::InventoryModel(std::shared_ptr<application::InventoryService> inventory_service,
                               QObject* parent)
    : QAbstractListModel(parent), m_inventory_service(std::move(inventory_service)) {
  if (!m_inventory_service) {
    throw std::invalid_argument("InventoryService cannot be null");
  }
  reload();
}

InventoryModel::~InventoryModel() = default;

auto InventoryModel::rowCount(const QModelIndex& parent) const -> int {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_mods.size());
}

auto InventoryModel::data(const QModelIndex& index, int role) const -> QVariant {
  if (!index.isValid() || index.row() >= static_cast<int>(m_mods.size()) || index.row() < 0) {
    return {};
  }

  if (role == Qt::DisplayRole) {
    return QString::fromStdString(m_mods[static_cast<std::size_t>(index.row())].name());
  }

  return {};
}

void InventoryModel::reload() {
  beginResetModel();
  m_mods = m_inventory_service->getInventory();
  endResetModel();
}

} // namespace ui
