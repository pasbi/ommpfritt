#include "mainwindow/toolbar.h"
#include <QToolButton>
#include "tools/toolbox.h"
#include "application.h"
#include "iconprovider.h"
#include <QApplication>

namespace
{

class StackToolButton : public QToolButton
{
public:
  explicit StackToolButton(QWidget* parent = nullptr) : QToolButton(parent)
  {
    connect(this, SIGNAL(triggered(QAction*)), this, SLOT(setDefaultAction(QAction*)));
  }
};

}  // namespace

namespace omm
{

ToolBar::ToolBar(const QString& tools)
{
  setAttribute(Qt::WA_DeleteOnClose);
  if (tools == "") {
     configure("undo;redo;Ellipse,RectangleObject;Instance");
  } else {
    configure(tools);
  }
}

void ToolBar::configure(const QString& tools)
{
  m_tools = tools;
  clear();
  if (!tools.isEmpty()) {
    auto& app = Application::instance();
    for (const QString token : tools.split(";")) {
      const QStringList tokens = token.split(",");
      if (tokens.size() == 1) {
        addAction(app.key_bindings.make_toolbar_action(app, token).release());
      } else {
        auto button = std::make_unique<StackToolButton>();
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        for (const QString& token : tokens) {
          button->addAction(app.key_bindings.make_menu_action(app, token).release());
        }
        button->setDefaultAction(button->actions().first());
        addWidget(button.release());
      }
    }
  }
}

}  // namespace omm
