#include "ui/InventoryWidget.hpp"

#include "ui/InventoryModel.hpp"

#include <QLineEdit>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

namespace fmm::ui {

InventoryWidget::InventoryWidget(InventoryModel* inventory_model, QWidget* parent)
    : QWidget(parent), m_inventory_model(inventory_model) {
  setupUi();
}

void InventoryWidget::setupUi() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  m_search_bar = new QLineEdit(this);
  m_search_bar->setPlaceholderText(QStringLiteral("Filter mods..."));
  layout->addWidget(m_search_bar);

  m_proxy_model = new QSortFilterProxyModel(this);
  m_proxy_model->setSourceModel(m_inventory_model);
  m_proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);

  connect(m_search_bar, &QLineEdit::textChanged, m_proxy_model,
          &QSortFilterProxyModel::setFilterFixedString);

  m_list_view = new QListView(this);
  m_list_view->setModel(m_proxy_model);
  layout->addWidget(m_list_view);
}

} // namespace fmm::ui
