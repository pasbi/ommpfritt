#include "mainwindow/mainwindow.h"

#include <memory>
#include <glog/logging.h>

#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>
#include <QSettings>
#include <QCloseEvent>
#include <functional>

#include "mainwindow/viewport/viewport.h"
#include "mainwindow/application.h"
#include "mainwindow/toolbar.h"
#include "managers/manager.h"

namespace
{

template<typename T, typename F>
auto read_each(QSettings& settings, const std::string& key, const F& f)
{
  auto size = settings.beginReadArray(QString::fromStdString(key));
  std::vector<T> ts;
  ts.reserve(size);
  for (decltype(size) i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    ts.push_back(f());
  }
  settings.endArray();
  return ts;
}

template<typename F>
void read_each(QSettings& settings, const std::string& key, const F& f)
{
  auto size = settings.beginReadArray(QString::fromStdString(key));
  for (decltype(size) i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    f();
  }
  settings.endArray();
}

template<typename Ts, typename F>
void write_each(QSettings& settings, const std::string& key, const Ts& ts, const F& f)
{
  settings.beginWriteArray(QString::fromStdString(key));
  size_t i = 0;
  for (auto&& t : ts) {
    settings.setArrayIndex(i);
    f(t);
    i += 1;
  }
  settings.endArray();
}

}  // namespace

namespace omm
{

std::map<std::string, std::list<std::string>> MainWindow::main_menu_entries()
{
  std::map<std::string, std::list<std::string>> entries = {
    { "file", { "new document", "save document", "save document as", "load document" } },
    { "edit", { "undo", "redo" } },
    { "path", { "make smooth", "make linear", "remove points", "subdivide", "select all",
                "deselect all", "invert selection" } },
    { "tool", { "previous tool", KeyBindings::SEPARATOR } },
    { "scene", { "evaluate" } },
    { "create", {} },
    { "window", { KeyBindings::SEPARATOR, "show keybindings dialog" } }
  };

  for (const std::string& key : Manager::keys()) { entries["window"].push_front("show " + key); }
  for (const std::string& key : Object::keys()) { entries["create"].push_back("create " + key); }
  for (const std::string& key : Tool::keys()) { entries["tool"].push_back("select " + key); }

  return entries;
}

MainWindow::MainWindow(Application& app)
  : m_app(app)
{
  setDockNestingEnabled(true);
  setCentralWidget(std::make_unique<Viewport>(app.scene).release());
  restore_state();

  setMenuBar(std::make_unique<QMenuBar>().release());
  add_menu("&File", main_menu_entries()["file"]);
  add_menu("&Edit", main_menu_entries()["edit"]);
  add_menu("&Create", main_menu_entries()["create"]);
  add_menu("&Path", main_menu_entries()["path"]);
  add_menu("&Tool", main_menu_entries()["tool"]);
  add_menu("&Scene", main_menu_entries()["scene"]);
  add_menu("&Window", main_menu_entries()["window"]);
}

void MainWindow::add_menu(const std::string& title, const std::list<std::string>& actions)
{
  auto menu = m_app.key_bindings.make_menu(m_app, actions);
  menu->setTearOffEnabled(true);
  menu->setTitle(QString::fromStdString(title));
  menuBar()->addMenu(menu.release());
}

void MainWindow::restore_state()
{
  LOG(INFO) << "restore-state";
  QSettings settings;
  restoreGeometry(settings.value("mainwindow/geometry").toByteArray());

  read_each(settings, "mainwindow/toolbars", [this, &settings]() {
    const auto tools = read_each<std::string>(settings, "tools", [&settings]() {
      return settings.value("tool").toString().toStdString();
    });
    auto tool_bar = std::make_unique<ToolBar>(this, m_app.scene.tool_box, tools);
    addToolBar(Qt::TopToolBarArea, tool_bar.release());
  });

  restoreState(settings.value("mainwindow/window_state").toByteArray());

  read_each(settings, "mainwindow/managers", [this, &settings]() {
    const auto type = settings.value("type").toString().toStdString();
    auto manager = Manager::make(type, m_app.scene);
    if (!restoreDockWidget(manager.release())) {
      LOG(WARNING) << "Failed to restore geometry of manager.";
    }
  });
}

void MainWindow::save_state()
{
  QSettings settings;
  settings.setValue("mainwindow/geometry", saveGeometry());
  settings.setValue("mainwindow/window_state", saveState());

  const auto save_manager = [&settings](const Manager* manager) {
    settings.setValue("type", QString::fromStdString(manager->type()));
  };
  write_each(settings, "mainwindow/managers", findChildren<Manager*>(), save_manager);

  const auto save_tool_bar = [&settings](const ToolBar* tool_bar) {
    write_each(settings, "tools", tool_bar->tools(), [&settings](const std::string& tool) {
      settings.setValue("tool", QString::fromStdString(tool));
    });
  };
  write_each(settings, "mainwindow/toolbars", findChildren<ToolBar*>(), save_tool_bar);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  save_state();
  QMainWindow::closeEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
  if (!Application::instance().key_bindings.call(*e, m_app)) {
    QMainWindow::keyPressEvent(e);
  }
}

}  // namespace omm