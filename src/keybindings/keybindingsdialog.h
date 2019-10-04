#pragma once
#include <QDialog>
#include "keybindings/keybindingsproxymodel.h"
#include <memory>

namespace Ui { class KeyBindingsDialog; }

namespace omm
{

class KeyBindings;

class KeyBindingsDialog : public QDialog
{
  Q_OBJECT
public:
  explicit KeyBindingsDialog(KeyBindings& key_bindings, QWidget* parent = nullptr);
  ~KeyBindingsDialog();

  static constexpr auto KEYBINDINGS_DIALOG_SETTINGS_GROUP = "keybindingsdialog";
  static constexpr auto GEOMETRY_SETTINGS_KEY = "geometry";

  void reject() override;
  void accept() override;

private:
  KeyBindingsProxyModel m_proxy_model;
  std::unique_ptr<Ui::KeyBindingsDialog> m_ui;
  KeyBindings& m_key_bindings;
  std::map<std::string, std::map<std::string, QKeySequence>> m_revert_cache;
  void update_expand();

private Q_SLOTS:
  void reset();
  void reset_filter();
  void set_name_filter(const QString& filter);
  void set_sequence_filter(const QKeySequence& sequence);
};

}  // namespace omm
