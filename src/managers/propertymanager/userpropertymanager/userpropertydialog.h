#pragma once

#include <QDialog>
#include "managers/propertymanager/userpropertymanager/userpropertylistmodel.h"

class QListView;

namespace omm
{

class AbstractPropertyOwner;
class UserPropertyDialog : public QDialog
{
public:
  explicit UserPropertyDialog(QWidget* parent);
  void set_property_owner(AbstractPropertyOwner* property);

  void new_user_property();
  void remove_user_property();

private:
  QListView* m_list_view;
  QWidget* m_right_column;
  void setup_right_column();
  std::unique_ptr<UserPropertyListModel> m_user_property_list_model;

};

}  // namespace omm
