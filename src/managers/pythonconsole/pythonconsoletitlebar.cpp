#include "managers/pythonconsole/pythonconsoletitlebar.h"
#include "managers/pythonconsole/pythonconsole.h"
#include "keybindings/menu.h"
#include "menuhelper.h"
#include <QLabel>
#include <QMenuBar>
#include "mainwindow/application.h"
#include "keybindings/keybindings.h"

namespace omm
{

PythonConsoleTitleBar::PythonConsoleTitleBar(PythonConsole& parent)
  : ManagerTitleBar(parent)
{
  auto python_menu = std::make_unique<Menu>(QObject::tr("Python", "PythonConsole"));

  auto& app = Application::instance();
  python_menu->addAction(app.key_bindings.make_menu_action(parent, "clear console").release());
  python_menu->addAction(app.key_bindings.make_menu_action(parent, "evaluate").release());

  auto menu_bar = std::make_unique<QMenuBar>();
  menu_bar->setSizePolicy(QSizePolicy::Preferred, menu_bar->sizePolicy().verticalPolicy());
  menu_bar->addMenu(python_menu.release());

  auto container = std::make_unique<QWidget>();
  auto layout = std::make_unique<QHBoxLayout>();
  layout->addWidget(menu_bar.release());
  layout->addStretch(0);
  layout->setContentsMargins(0, 0, 0, 0);
  container->setLayout(layout.release());
  add_widget(std::move(container));
}

}  // namespace omm
