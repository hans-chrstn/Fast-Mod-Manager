#include "ui/ProgressOverlayWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

namespace fmm::ui {

ProgressOverlayWidget::ProgressOverlayWidget(QWidget* parent) : QWidget(parent) { setupUi(); }

void ProgressOverlayWidget::setupUi() {
  auto* progress_layout = new QHBoxLayout(this);
  progress_layout->setContentsMargins(0, 0, 0, 0);

  m_status_label = new QLabel(QStringLiteral("Ready"), this);
  progress_layout->addWidget(m_status_label);

  m_progress_bar = new QProgressBar(this);
  constexpr int max_progress = 100;
  m_progress_bar->setRange(0, max_progress);
  m_progress_bar->setValue(0);
  progress_layout->addWidget(m_progress_bar);

  m_cancel_button = new QPushButton(QStringLiteral("Cancel"), this);
  connect(m_cancel_button, &QPushButton::clicked, this, &ProgressOverlayWidget::cancelRequested);
  progress_layout->addWidget(m_cancel_button);
}

void ProgressOverlayWidget::setProgress(int percentage, const QString& message) {
  m_progress_bar->setValue(percentage);
  m_status_label->setText(message);
}

void ProgressOverlayWidget::resetState() {
  m_progress_bar->setValue(0);
  m_status_label->setText(QStringLiteral("Ready"));
}

void ProgressOverlayWidget::hideOverlay() { hide(); }

void ProgressOverlayWidget::showOverlay() { show(); }

} // namespace fmm::ui
