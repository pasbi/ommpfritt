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
#include "mainwindow/mainmenubar.h"
#include "mainwindow/application.h"
#include "mainwindow/toolbar.h"

#include "managers/propertymanager/propertymanager.h"
#include "managers/objectmanager/objectmanager.h"
#include "scene/scene.h"
#include "commands/addcommand.h"
#include "objects/path.h"
#include "mainwindow/actions.h"
#include "common.h"
#include "keybindings/keybindingsdialog.h"

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

constexpr auto SEPARATOR = "separator";
const std::vector<std::string> file_menu_actions = { "new document", "save document",
  "save document as", "load document" };

const std::vector<std::string> edit_menu_actions = { "undo", "redo" };
const std::vector<std::string> path_menu_actions = { "make smooth", "make linear", "remove points",
   "subdivide" };
const std::vector<std::string> scene_menu_actions = { "evaluate" };
const std::vector<std::string> create_menu_actions()
{
  return ::transform<std::string, std::vector>(omm::Object::keys(), [](const auto& key) {
    return "create " + key;
  });
};
const std::vector<std::string> window_menu_actions()
{
  auto actions = ::transform<std::string, std::vector>(omm::Manager::keys(), [](const auto& key) {
    return "show " + key;
  });
  actions.push_back(SEPARATOR);
  actions.push_back("show keybindings dialog");
  return actions;
};



}  // namespace

namespace omm
{

MainWindow::MainWindow(Application& app)
  : m_app(app)
{
  setDockNestingEnabled(true);
  setCentralWidget(std::make_unique<Viewport>(app.scene).release());
  restore_state();

  setMenuBar(std::make_unique<QMenuBar>().release());
  add_menu("&File", file_menu_actions);
  add_menu("&Edit", edit_menu_actions);
  add_menu("&Create", create_menu_actions());
  add_menu("&Scene", scene_menu_actions);
  add_menu("&Window", window_menu_actions());
}

void MainWindow::add_menu(const std::string& title, const std::vector<std::string>& actions)
{
  auto menu = std::make_unique<QMenu>();
  for (const auto action_name : actions) {
    if (action_name == SEPARATOR) {
      menu->addSeparator();
    } else {
      auto action = m_app.key_bindings.make_action(*this, action_name);
      if (action != nullptr) {
        action->setParent(this);
        menu->addAction(action.release());
      }
    }
  }
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

std::map<std::string, QKeySequence> MainWindow::default_bindings()
{
  std::map<std::string, QKeySequence> map {
    { "undo", QKeySequence("Ctrl+Z") },
    { "redo", QKeySequence("Ctrl+Y") },
    { "new document", QKeySequence("Ctrl+N") },
    { "save document", QKeySequence("Ctrl+S") },
    { "save document as", QKeySequence("Ctrl+Shift+S") },
    { "load document", QKeySequence("Ctrl+O") },
    { "make smooth", QKeySequence() },
    { "make linear", QKeySequence() },
    { "remove points", QKeySequence() },
    { "subdivide", QKeySequence() },
    { "evaluate", QKeySequence() },
    { "show keybindings dialog", QKeySequence() },
  };

  for (const auto& key : Object::keys()) {
    map.insert(std::pair("create " + key, QKeySequence()));
  }

  for (const auto& key : Manager::keys()) {
    map.insert(std::pair("show " + key, QKeySequence()));
  }

  return map;
}

void MainWindow::call(const std::string& command)
{
  const auto save = static_cast<bool(Application::*)()>(&Application::save);

  Dispatcher map {
    { "undo", std::bind(&QUndoStack::undo, &m_app.scene.undo_stack) },
    { "redo", std::bind(&QUndoStack::redo, &m_app.scene.undo_stack) },
    { "new document", std::bind(&Application::reset, &m_app) },
    { "save document", std::bind(save, &m_app) },
    { "save document as", std::bind(&Application::save_as, &m_app) },
    { "load document", std::bind(&Application::load, &m_app) },
    { "make smooth", std::bind(&actions::make_smooth, std::ref(m_app.scene)) },
    { "make linear", std::bind(&actions::make_linear, std::ref(m_app.scene)) },
    { "remove points", std::bind(&actions::remove_selected_points, std::ref(m_app.scene)) },
    { "subdivide", std::bind(&actions::subdivide, std::ref(m_app.scene)) },
    { "evaluate", std::bind(&actions::evaluate, std::ref(m_app.scene)) },
    { "show keybindings dialog", [this]() {
        KeyBindingsDialog(Application::instance().key_bindings, this).exec();
    } },
  };

  for (const auto& key : Object::keys()) {
    map.insert(std::pair("create " + key, [this, key](){
      Scene& scene = m_app.scene;
      using add_command_type = AddCommand<Tree<Object>>;
      scene.submit<add_command_type>(scene.object_tree, Object::make(key, &scene));
    }));
  }

  for (const auto& key : Manager::keys()) {
    map.insert(std::pair("show " + key, [this, key](){
      auto manager = Manager::make(key, m_app.scene);
      auto& ref = *manager;
      addDockWidget(Qt::TopDockWidgetArea, manager.release());
      ref.setFloating(true);
    }));
  }

  dispatch(command, map);
}

std::string MainWindow::type() const { return TYPE; }
void MainWindow::keyPressEvent(QKeyEvent* e)
{
  if (!Application::instance().key_bindings.call(*e, *this)) {
    QMainWindow::keyPressEvent(e);
  }
}

}  // namespace omm