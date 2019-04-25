#pragma once
#include <QDialog>
#include "keybindings/keybindingsproxymodel.h"

namespace omm
{

class KeyBindings;

class KeyBindingsDialog : public QDialog
{
public:
  explicit KeyBindingsDialog(KeyBindings& key_bindings, QWidget* parent = nullptr);
  ~KeyBindingsDialog();

  static constexpr auto KEYBINDINGS_DIALOG_SETTINGS_GROUP = "keybindingsdialog";
  static constexpr auto GEOMETRY_SETTINGS_KEY = "geometry";

private:
  KeyBindingsProxyModel m_proxy_model;
};

}  // namespace omm
