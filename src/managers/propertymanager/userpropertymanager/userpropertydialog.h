#pragma once

#include <QDialog>
#include "commands/userpropertyconfigcommand.h"
#include <memory>
#include "ui_userpropertydialog.h"
#include "managers/propertymanager/userpropertymanager/userpropertylistmodel.h"

namespace Ui { class UserPropertyDialog; }

namespace omm
{

class PropertyConfigWidget;

struct PropertyItem : public std::map<std::string, QVariant>
{

};

class UserPropertyDialog: public QDialog
{
  Q_OBJECT
public:
  explicit UserPropertyDialog(AbstractPropertyOwner& owner, QWidget* parent = nullptr);

public Q_SLOTS:
  void submit();

private:
  std::unique_ptr<Ui::UserPropertyDialog> m_ui;
  const std::vector<std::string> m_property_types;
  AbstractPropertyOwner& m_owner;
  UserPropertyListModel m_user_property_list_model;
  UserPropertyListItem* m_current_item = nullptr;
  PropertyConfigWidget* m_current_config_widget = nullptr;
  void update_property_config_page(UserPropertyListItem *item);

};


}  // namespace omm
