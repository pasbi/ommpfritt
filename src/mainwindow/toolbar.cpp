#include "mainwindow/toolbar.h"
#include "tools/toolbox.h"
#include "application.h"
#include "iconprovider.h"

#include <QApplication>

namespace omm
{

ToolBar::ToolBar()
{
  setAttribute(Qt::WA_DeleteOnClose);

  auto& app = Application::instance();
  auto action = app.key_bindings.make_toolbar_action(app, "undo");
  connect(action.get(), &QAction::triggered, []() {
    LINFO << "undoS{!";
  });
  addAction(action.release());
  connect(addAction("Hello!"), &QAction::triggered, []() {
    LINFO << "hello!";
  });
}

}  // namespace omm
