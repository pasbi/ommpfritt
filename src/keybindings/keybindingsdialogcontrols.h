#pragma once

#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;

namespace omm
{

class KeyBindingsProxyModel;

class KeyBindingsDialogControls : public QWidget
{
  Q_OBJECT

public:
  explicit KeyBindingsDialogControls(KeyBindingsProxyModel& filter_proxy);

private:
  KeyBindingsProxyModel& m_filter_proxy;
  QComboBox* m_context_filter_combobox;
  QLineEdit* m_filter_line_edit;
  QPushButton* m_clear_button;
};

}  // namespace omm
