#pragma once

#include "application/InventoryError.hpp"
#include "domain/InstalledPackage.hpp"

#include <QAbstractListModel>
#include <QObject>
#include <expected>
#include <memory>
#include <string>
#include <vector>

namespace fmm::application {
class InventoryService;
}

namespace fmm::ui {

template <typename T> class AsyncTask;

class InventoryModel : public QAbstractListModel {
  Q_OBJECT

public:
  explicit InventoryModel(std::shared_ptr<fmm::application::InventoryService> inventory_service,
                          QObject* parent = nullptr);
  ~InventoryModel() override;

  [[nodiscard]] auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;
  [[nodiscard]] auto data(const QModelIndex& index, int role = Qt::DisplayRole) const
      -> QVariant override;

  void reload();
  void cancelReload();

private:
  std::shared_ptr<fmm::application::InventoryService> m_inventory_service;
  std::vector<fmm::domain::InstalledPackage> m_mods;
  std::unique_ptr<AsyncTask<
      std::expected<std::vector<fmm::domain::InstalledPackage>, fmm::application::InventoryError>>>
      m_load_task;

Q_SIGNALS:
  void scanProgress(int percentage, const QString& message);
  void scanCompleted();
  void scanFailed(const QString& error);
};

} // namespace fmm::ui
