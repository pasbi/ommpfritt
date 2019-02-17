#pragma once
#include <QDialog>

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
};

}  // namespace omm
