#pragma once

#include <QMainWindow>

namespace ui {
class InventoryWidget;
class ProgressOverlayWidget;
class InventoryModel;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(InventoryModel* inventory_model, QWidget* parent = nullptr);
  ~MainWindow() override = default;

Q_SIGNALS:
  void requestDependencyCheck();

private:
  void setupUi();

  InventoryModel* m_inventory_model;
  InventoryWidget* m_inventory_widget{nullptr};
  ProgressOverlayWidget* m_progress_overlay{nullptr};

  Q_SLOT void onScanProgress(int percentage, const QString& message);
  Q_SLOT void onScanCompleted();
  Q_SLOT void onScanFailed(const QString& error);
  Q_SLOT void onCancelClicked();
};

} // namespace ui
