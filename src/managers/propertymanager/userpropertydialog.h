#pragma once

#include <QDialog>
#include "commands/userpropertyconfigcommand.h"
#include <memory>
#include "ui_userpropertydialog.h"
#include "managers/propertymanager/userpropertylistmodel.h"

namespace Ui { class UserPropertyDialog; }

namespace omm
{

class PropertyConfigWidget;

class UserPropertyDialog: public QDialog
{
  Q_OBJECT
public:
  explicit UserPropertyDialog(AbstractPropertyOwner& owner, const std::set<QString>& disabled_types,
                              QWidget* parent = nullptr);
public Q_SLOTS:
  void submit();

private:
  std::unique_ptr<Ui::UserPropertyDialog> m_ui;
  const std::vector<QString> m_property_types;
  AbstractPropertyOwner& m_owner;
  UserPropertyListModel m_user_property_list_model;
  UserPropertyListItem* m_current_item = nullptr;
  PropertyConfigWidget* m_current_config_widget = nullptr;
  void update_property_config_page(UserPropertyListItem *item);

};


}  // namespace omm
