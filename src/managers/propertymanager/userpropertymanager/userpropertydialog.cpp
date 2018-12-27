#include "managers/propertymanager/userpropertymanager/userpropertydialog.h"
#include <memory>
#include <functional>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QMenuBar>
#include "menuhelper.h"
#include "managers/propertymanager/userpropertymanager/propertyitem.h"
#include "managers/propertymanager/userpropertymanager/propertyconfigwidget.h"


namespace omm
{

UserPropertyDialog::UserPropertyDialog(QWidget* parent, AbstractPropertyOwner& property)
  : QDialog(parent)
{
  setLayout(std::make_unique<QHBoxLayout>(this).release());
  layout()->setMenuBar(std::make_unique<QMenuBar>(this).release());
  m_list_widget = std::make_unique<QListWidget>(this).release();
  layout()->addWidget(m_list_widget);
  m_right_column = nullptr;

  QMenuBar& menu_bar = static_cast<QMenuBar&>(*layout()->menuBar());
  QMenu& user_property_menu = *menu_bar.addMenu("User Properties");

  action( user_property_menu, "New User Property", *this,
          &UserPropertyDialog::new_item );
  action( user_property_menu, "Remove User Property", *this,
          &UserPropertyDialog::remove_selected_item );
  connect(m_list_widget, &QListWidget::currentItemChanged, [this](auto* item) {
    on_current_item_changed(static_cast<PropertyItem*>(item));
  });
}

void UserPropertyDialog::new_item()
{
  m_list_widget->addItem(std::make_unique<PropertyItem>().release());
}

void UserPropertyDialog::remove_selected_item()
{

}

void UserPropertyDialog::on_current_item_changed(QListWidgetItem* item)
{
  using f_type = std::function<void(UserPropertyDialog&, PropertyItem*)>;
  using map_type = std::map<std::string, f_type>;

  const auto* property_item = static_cast<PropertyItem*>(item);
  const auto class_name = property_item->property().type() + "ConfigWidget";

  delete m_right_column;
  LOG(INFO) << "make " << class_name;
  m_right_column = AbstractPropertyConfigWidget::make(class_name, this).release();
  layout()->takeAt(1);
  layout()->addWidget(m_right_column);
}

}  // namespace omm
