#include "managers/propertymanager/userpropertymanager/userpropertydialog.h"
#include "aspects/propertyowner.h"
#include "logging.h"
#include "propertywidgets/propertyconfigwidget.h"
#include <iomanip>
#include <ostream>

namespace omm
{

UserPropertyDialog::UserPropertyDialog(AbstractPropertyOwner &owner, QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::UserPropertyDialog)
  , m_property_types(::transform<std::string, std::vector>(Property::keys()))
  , m_owner(owner)
  , m_user_property_list_model(owner)
{
  m_ui->setupUi(this);
  m_ui->listView->setModel(&m_user_property_list_model);
  connect(m_ui->listView->selectionModel(), &QItemSelectionModel::currentChanged,
          [this](const QModelIndex& index)
  {
    update_property_config_page(m_user_property_list_model.item(index));
    m_ui->listView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
  });

  m_ui->cb_type->addItems(::transform<QString, QList>(m_property_types, [](const std::string& s)
  {
    return tr(s.c_str(), "Property");
  }));
  connect(m_ui->cb_type, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
    m_current_item->configuration["type"] = m_property_types[index];
    update_property_config_page(m_current_item);
  });

  connect(m_ui->pb_add, &QPushButton::clicked, [this]() {
    m_user_property_list_model.add_property(m_ui->cb_type->currentText());
    const int n = m_user_property_list_model.rowCount(QModelIndex());
    const QModelIndex index = m_user_property_list_model.index(n-1, 0);
    m_ui->listView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    m_ui->listView->edit(index);
  });
  connect(m_ui->pb_del, &QPushButton::clicked, [this]() {
    m_user_property_list_model.del_property(m_ui->listView->currentIndex());
  });
  connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(submit()));
  m_ui->cb_type->hide();
}

void UserPropertyDialog::submit()
{
  const auto keys = m_owner.properties().keys();
  for (const std::string& property_key : keys) {
    Property* p = m_owner.property(property_key);
    if (p != nullptr && p->is_user_property() && !m_user_property_list_model.contains(p)) {
      m_owner.extract_property(property_key);
    }
  }
  for (const UserPropertyListItem* item : m_user_property_list_model.items()) {
    if (item->property() != nullptr) {
      item->property()->configure(item->configuration);
    } else {
      auto property = Property::make(item->type());
      property->configure(item->configuration);
      property->set_label(item->label());
      property->set_category(Property::USER_PROPERTY_CATEGROY_NAME);
      std::string key = item->label();
      if (key.empty()) {
        key = tr("unnamed").toStdString();
      }
      for (int i = 0; m_owner.has_property(key); ++i) {
        std::ostringstream ostream;
        ostream << item->label() << "." << std::setw(3) << std::setfill('0') << i;
        key = ostream.str();
      }
      m_owner.add_property(key, std::move(property));
    }
  }
}

void UserPropertyDialog::update_property_config_page(UserPropertyListItem* item)
{
  if (m_current_config_widget != nullptr && m_current_item != nullptr) {
    m_current_config_widget->update(m_current_item->configuration);
  }

  if (QWidget* current_widget = m_ui->scrollArea->widget(); current_widget != nullptr) {
    current_widget->deleteLater();
  }

  m_current_item = item;

  if (m_current_item == nullptr) {
    m_ui->scrollArea->setWidget(std::make_unique<QWidget>().release());
    m_ui->cb_type->hide();
  } else {
    const std::string type = m_current_item->type();
    {
      QSignalBlocker blocker(m_ui->cb_type);
      m_ui->cb_type->setEnabled(m_current_item->property() == nullptr);
      m_ui->cb_type->setCurrentText(QString::fromStdString(type));
    }

    const std::string config_widget_type = type + "ConfigWidget";
    auto config_widget = PropertyConfigWidget::make(config_widget_type);
    connect(config_widget.get(), &PropertyConfigWidget::hidden,
            [this, config_widget=config_widget.get()]()
    {
      UserPropertyListItem* item = m_user_property_list_model.item(m_ui->listView->currentIndex());
      config_widget->update(item->configuration);
    });
    m_current_config_widget = config_widget.get();
    config_widget->init(m_current_item->configuration);
    m_ui->scrollArea->setWidget(config_widget.release());
    m_ui->cb_type->show();
  }
}

}  // namespace omm
