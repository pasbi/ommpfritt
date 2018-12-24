#include "managers/propertymanager/userpropertymanager/userpropertydialog.h"
#include <memory>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QListView>
#include <QLineEdit>
#include <QMenuBar>
#include "menuhelper.h"


namespace omm
{

UserPropertyDialog::UserPropertyDialog(QWidget* parent)
  : QDialog(parent)
{
  setLayout(std::make_unique<QHBoxLayout>(this).release());
  layout()->setMenuBar(std::make_unique<QMenuBar>(this).release());
  m_list_view = std::make_unique<QListView>(this).release();
  layout()->addWidget(m_list_view);
  m_right_column = nullptr;

  QMenuBar& menu_bar = static_cast<QMenuBar&>(*layout()->menuBar());
  QMenu& user_property_menu = *menu_bar.addMenu("User Properties");
  action( user_property_menu, "New User Property", *this,
          &UserPropertyDialog::new_user_property );
  action( user_property_menu, "Remove User Property", *this,
          &UserPropertyDialog::remove_user_property );
}

void UserPropertyDialog::setup_right_column()
{
  delete m_right_column;
  m_right_column = std::make_unique<QWidget>(this).release();
  layout()->takeAt(1);
  layout()->addWidget(m_right_column);

  auto form_layout = std::make_unique<QFormLayout>(m_right_column).release();
  m_right_column->setLayout(form_layout);

  form_layout->addRow("Name", std::make_unique<QLineEdit>(m_right_column).release());
}

void UserPropertyDialog::set_property_owner(AbstractPropertyOwner* property)
{
  setup_right_column();
  m_user_property_list_model = std::make_unique<UserPropertyListModel>(*property);
  m_list_view->setModel(m_user_property_list_model.get());
}

void UserPropertyDialog::new_user_property()
{
  // m_user_property_list_model.
}

void UserPropertyDialog::remove_user_property()
{

}

}  // namespace omm
