#pragma once

#include <QDialog>
#include <memory>

namespace Ui { class ToolBarDialog; }

class QIdentityProxyModel;

namespace omm
{

class KeyBindings;
class KeyBindingsProxyModel;
class ToolBarItemModel;

class ToolBarDialog : public QDialog
{
public:
  ToolBarDialog(const QString& tools, QWidget* parent = nullptr);
  ~ToolBarDialog();
  QString tools() const;
  static constexpr auto mime_type = "application/command";

private:
  KeyBindings& m_key_bindings;
  std::unique_ptr<Ui::ToolBarDialog> m_ui;
  std::unique_ptr<KeyBindingsProxyModel> m_proxy;;
  std::unique_ptr<ToolBarItemModel> m_toolbar_item_model;
};

}
