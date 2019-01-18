#include "mainmenubar.h"

#include <memory>
#include <QAction>
#include <QMenu>
#include <QApplication>

#include "mainwindow/mainwindow.h"
#include "mainwindow/application.h"
#include "commands/addcommand.h"
#include "managers/manager.h"
#include "menuhelper.h"
#include "mainwindow/toolbar.h"
#include "common.h"
#include "objects/path.h"
#include "mainwindow/pathmenu.h"
#include "tags/scripttag.h"

namespace omm
{

void MainMenuBar::make_file_menu()
{
  auto& file_menu = *addMenu(tr("&File"));
  action(file_menu, tr("&new"), m_app, &Application::reset);
  action(file_menu, tr("&load"), m_app, &Application::load);
  action(file_menu, tr("&save"), m_app, static_cast<bool (Application::*)()>(&Application::save));
  action(file_menu, tr("&save_as"), m_app, &Application::save_as);
}

void MainMenuBar::make_create_menu()
{
  auto& create_menu = *addMenu(tr("&Create"));
  for (const auto& key : Object::keys())
  {
    action(create_menu, QString::fromStdString(key), [this, key]() {
      Scene& scene = m_app.scene;
      using add_command_type = AddCommand<Tree<Object>>;
      scene.submit<add_command_type>(scene.object_tree, Object::make(key, &scene));
    });
  }
}

void MainMenuBar::make_scene_menu()
{
  auto& scene_menu = *addMenu(tr("&Scene"));
  action(scene_menu, tr("&evaluate"), [this]() {
    for (Tag* tag : m_app.scene.tags()) {
      auto* script_tag = type_cast<ScriptTag*>(tag);
      if (script_tag != nullptr) { script_tag->force_evaluate(); }
    }
  });
}

void MainMenuBar::make_window_menu()
{
  auto& window_menu = *addMenu(tr("&Window"));
  for (auto& key : Manager::keys()) {
    action(window_menu, QString::fromStdString(key), [this, key]() {
      auto manager = Manager::make(key, m_app.scene);
      auto& ref = *manager;
      m_main_window.addDockWidget(Qt::TopDockWidgetArea, manager.release());
      ref.setFloating(true);
    });
  }
  action(window_menu, "toolbar", [this]() {
    auto keys = Tool::keys();
    const auto all_tools = ::transform<std::string, std::vector>(keys, ::identity);
    auto tool_bar = std::make_unique<ToolBar>(this, m_app.scene.tool_box, all_tools);
    m_main_window.addToolBar(Qt::TopToolBarArea, tool_bar.release());
    // TODO set floating true
  });
}

void MainMenuBar::make_edit_menu()
{
  auto& edit_menu = *addMenu(tr("&Edit"));
  edit_menu.addAction(m_app.scene.undo_stack.createUndoAction(nullptr));
  edit_menu.addAction(m_app.scene.undo_stack.createRedoAction(nullptr));
}

void MainMenuBar::make_path_menu()
{
  addMenu(std::make_unique<PathMenu>(m_app.scene, this).release());
}

MainMenuBar::MainMenuBar(Application& app, MainWindow& main_window)
  : m_app(app)
  , m_main_window(main_window)
{
  make_file_menu();
  make_edit_menu();
  make_create_menu();
  make_path_menu();
  make_scene_menu();
  make_window_menu();
}

}  // namespace omm
