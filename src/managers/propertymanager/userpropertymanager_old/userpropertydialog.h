//#pragma once

//#include <QDialog>
//#include <memory>
//#include "commands/userpropertyconfigcommand.h"

//class QListWidget;
//class QListWidgetItem;
//class QHBoxLayout;

//namespace omm
//{

//class AbstractPropertyConfigWidget;
//class AbstractPropertyOwner;
//class PropertyItem;

//class UserPropertyDialog : public QDialog
//{
//public:
//  explicit UserPropertyDialog(QWidget* parent, AbstractPropertyOwner& property_owner);
//  std::unique_ptr<UserPropertyConfigCommand> make_user_property_config_command() const;

//private:
//  QListWidget* m_list_widget;
//  AbstractPropertyConfigWidget* m_right_column;
//  void new_item();
//  void remove_selected_item();
//  void on_current_item_changed(QListWidgetItem*);
//  void on_current_item_type_changed(const std::string& type);
//  void on_current_item_label_changed();
//  PropertyItem* current_item() const;
//  QHBoxLayout* m_layout;
//  AbstractPropertyOwner& m_property_owner;
//};

//}  // namespace omm
