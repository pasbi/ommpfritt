#include "managers/propertymanager/userpropertymanager/userpropertydialog.h"
#include <memory>
#include <functional>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMenuBar>
#include "menuhelper.h"
#include "managers/propertymanager/userpropertymanager/propertyitem.h"
#include "propertywidgets/propertyconfigwidget.h"
#include "aspects/propertyowner.h"


namespace omm
{

UserPropertyDialog::UserPropertyDialog(QWidget* parent, AbstractPropertyOwner& property_owner)
  : QDialog(parent)
  , m_property_owner(property_owner)
{
  auto* main_layout = std::make_unique<QVBoxLayout>(this).release();
  main_layout->setMenuBar(std::make_unique<QMenuBar>(this).release());

  m_layout = std::make_unique<QHBoxLayout>().release();
  main_layout->addLayout(m_layout);

  m_list_widget = std::make_unique<QListWidget>(this).release();
  m_layout->addWidget(m_list_widget);
  m_list_widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

  m_right_column = nullptr;

  QMenuBar& menu_bar = static_cast<QMenuBar&>(*main_layout->menuBar());
  QMenu& user_property_menu = *menu_bar.addMenu("User Properties");

  action( user_property_menu, "New User Property", *this,
          &UserPropertyDialog::new_item );
  action( user_property_menu, "Remove User Property", *this,
          &UserPropertyDialog::remove_selected_item );
  connect(m_list_widget, &QListWidget::currentItemChanged, [this](auto* item) {
    on_current_item_changed(static_cast<PropertyItem*>(item));
  });

  const auto buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
  auto* button_box = std::make_unique<QDialogButtonBox>(buttons, this).release();
  connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
  connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));
  main_layout->addWidget(button_box);

  for (auto&& property : property_owner.properties().values()) {
    if (property->is_user_property()) {
      m_list_widget->addItem(std::make_unique<PropertyItem>(property->clone()).release());
    }
  }
}

void UserPropertyDialog::new_item()
{
  auto* const item = std::make_unique<PropertyItem>().release();
  m_list_widget->addItem(item);
  m_list_widget->setCurrentItem(item);
}

void UserPropertyDialog::remove_selected_item()
{

}

void UserPropertyDialog::on_current_item_changed(QListWidgetItem* item)
{
  using f_type = std::function<void(UserPropertyDialog&, PropertyItem*)>;
  using map_type = std::map<std::string, f_type>;

  m_right_column->deleteLater();
  m_right_column = nullptr;
  const auto current_item = this->current_item();
  if (current_item != nullptr) {
    const auto property_type = current_item->property().type();
    const auto pcwt = property_type + "ConfigWidget";
    auto& current_property = current_item->property();
    m_right_column = AbstractPropertyConfigWidget::make(pcwt, this, current_property).release();
    m_layout->takeAt(1);
    m_layout->addWidget(m_right_column);
    connect( m_right_column, &AbstractPropertyConfigWidget::property_type_changed,
             this, &UserPropertyDialog::on_current_item_type_changed );
    connect( m_right_column, &AbstractPropertyConfigWidget::property_label_changed,
             this, &UserPropertyDialog::on_current_item_label_changed);

  }
}

void UserPropertyDialog::on_current_item_label_changed()
{
  current_item()->setText(QString::fromStdString(current_item()->property().label()));
}

void UserPropertyDialog::on_current_item_type_changed(const std::string& type)
{
  current_item()->set_property_type(type);
  on_current_item_changed(current_item());
}

PropertyItem* UserPropertyDialog::current_item() const
{
  return static_cast<PropertyItem*>(m_list_widget->currentItem());
}

std::unique_ptr<UserPropertyConfigCommand>
UserPropertyDialog::make_user_property_config_command() const
{
  std::vector<std::unique_ptr<Property>> properties;
  properties.reserve(m_list_widget->count());
  for (std::size_t i = 0; i < m_list_widget->count(); ++i) {
    properties.push_back(static_cast<PropertyItem*>(m_list_widget->item(i))->property().clone());
  }
  return std::make_unique<UserPropertyConfigCommand>(m_property_owner, std::move(properties));
}

}  // namespace omm
