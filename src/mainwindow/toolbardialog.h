#pragma once

#include <QDialog>
#include <memory>

namespace Ui { class ToolBarDialog; }


namespace omm
{

class KeyBindings;
class KeyBindingsProxyModel;

class ToolBarDialog : public QDialog
{
public:
  ToolBarDialog(const QString& tools, QWidget* parent = nullptr);
  ~ToolBarDialog();
  QString tools() const { return m_tools; }

private:
  QString m_tools;
  KeyBindings& m_key_bindings;
  std::unique_ptr<Ui::ToolBarDialog> m_ui;
  std::unique_ptr<KeyBindingsProxyModel> m_filter_proxy;
};

}
