#pragma once

#include <QWidget>

class QLabel;
class QProgressBar;
class QPushButton;

namespace ui {

class ProgressOverlayWidget : public QWidget {
  Q_OBJECT

public:
  explicit ProgressOverlayWidget(QWidget* parent = nullptr);
  ~ProgressOverlayWidget() override = default;

  void setProgress(int percentage, const QString& message);
  void resetState();
  void hideOverlay();
  void showOverlay();

Q_SIGNALS:
  void cancelRequested();

private:
  void setupUi();

  QLabel* m_status_label{nullptr};
  QProgressBar* m_progress_bar{nullptr};
  QPushButton* m_cancel_button{nullptr};
};

} // namespace ui
