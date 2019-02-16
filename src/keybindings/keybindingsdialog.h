#pragma once
#include <QDialog>

namespace omm
{

class KeyBindings;

class KeyBindingsDialog : public QDialog
{
public:
  explicit KeyBindingsDialog(KeyBindings& key_bindings, QWidget* parent = nullptr);
};

}  // namespace omm
