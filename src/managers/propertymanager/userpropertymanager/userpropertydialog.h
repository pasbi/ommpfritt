#pragma once

#include <QDialog>

class QListWidget;
class QListWidgetItem;

namespace omm
{

class AbstractPropertyOwner;
class PropertyItem;

class UserPropertyDialog : public QDialog
{
public:
  explicit UserPropertyDialog(QWidget* parent, AbstractPropertyOwner& property);
  void set_property_owner();

private:
  QListWidget* m_list_widget;
  QWidget* m_right_column;
  void generate_items(AbstractPropertyOwner& property);
  void new_item();
  void remove_selected_item();
  void on_current_item_changed(QListWidgetItem* item);
};

}  // namespace omm
