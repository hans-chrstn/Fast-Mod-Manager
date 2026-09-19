#include "ui/InventoryModel.hpp"

#include "application/InventoryService.hpp"
#include "ui/AsyncTask.hpp"

#include <QPointer>
#include <stdexcept>

namespace ui {

InventoryModel::InventoryModel(std::shared_ptr<application::InventoryService> inventory_service,
                               QObject* parent)
    : QAbstractListModel(parent), m_inventory_service(std::move(inventory_service)) {
  if (!m_inventory_service) {
    throw std::invalid_argument("InventoryService cannot be null");
  }
}

InventoryModel::~InventoryModel() { cancelReload(); }

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

void InventoryModel::cancelReload() {
  if (m_load_task) {
    m_load_task->cancel();
    m_load_task.reset();
  }
}

void InventoryModel::reload() {
  cancelReload();

  QPointer<InventoryModel> safe_this(this);
  m_load_task = std::make_unique<AsyncTask<std::vector<fmm::domain::ModIdentity>>>(
      this,
      [svc = m_inventory_service,
       safe_this](const std::stop_token& stoken) -> std::vector<fmm::domain::ModIdentity> {
        return svc->getInventory(
            stoken, [safe_this](int percentage, const std::string& message) -> void {
              if (safe_this) {
                QMetaObject::invokeMethod(
                    safe_this,
                    [safe_this, percentage, msg = QString::fromStdString(message)]() -> void {
                      if (safe_this) {
                        emit safe_this->scanProgress(percentage, msg);
                      }
                    },
                    Qt::QueuedConnection);
              }
            });
      },
      [this](const std::vector<fmm::domain::ModIdentity>& mods) -> void {
        beginResetModel();
        m_mods = mods;
        endResetModel();
        m_load_task.reset();
        emit scanCompleted();
      },
      [this](const std::string& err) -> void {
        m_load_task.reset();
        emit scanFailed(QString::fromStdString(err));
      });
}

} // namespace ui
