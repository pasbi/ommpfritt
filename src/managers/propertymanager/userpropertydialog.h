#pragma once

#include "managers/propertymanager/userpropertylistmodel.h"
#include "ui_userpropertydialog.h"
#include <QDialog>
#include <memory>

namespace Ui
{
class UserPropertyDialog;
}

namespace omm
{
class AbstractPropertyConfigWidget;

class UserPropertyDialog : public QDialog
{
  Q_OBJECT
public:
  explicit UserPropertyDialog(AbstractPropertyOwner& owner,
                              const std::set<QString>& types,
                              QWidget* parent = nullptr);

public:
  void submit();

private:
  std::unique_ptr<Ui::UserPropertyDialog> m_ui;
  const std::vector<QString> m_property_types;
  AbstractPropertyOwner& m_owner;
  UserPropertyListModel m_user_property_list_model;
  UserPropertyListItem* m_current_item = nullptr;
  AbstractPropertyConfigWidget* m_current_config_widget = nullptr;
  void update_property_config_page(UserPropertyListItem* item);
};

}  // namespace omm
