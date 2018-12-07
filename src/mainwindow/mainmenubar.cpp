#include "mainmenubar.h"

#include <memory>
#include <QAction>
#include <QMenu>
#include <QApplication>

#include "mainwindow/mainwindow.h"
#include "mainwindow/application.h"
#include "commands/addobjectcommand.h"
#include "managers/manager.h"
#include "menuhelper.h"

namespace omm
{

void MainMenuBar::make_file_menu()
{
  auto& file_menu = *addMenu(tr("&File"));
  action(file_menu, tr("&new"), m_app, &Application::new_scene);
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
      m_app.scene().submit<AddObjectCommand>(m_app.scene().object_tree, Object::make(key));
    });
  }
}

void MainMenuBar::make_window_menu()
{
  auto& window_menu = *addMenu(tr("&Window"));
  for (auto& key : Manager::keys()) {
    action(window_menu, QString::fromStdString(key), [this, key]() {
      auto manager = Manager::make(key, m_app.scene());
      auto& ref = *manager;
      m_main_window.addDockWidget(Qt::TopDockWidgetArea, manager.release());
      ref.setFloating(true);
    });
  }
}

void MainMenuBar::make_edit_menu()
{
  auto& edit_menu = *addMenu(tr("&Edit"));

  QUndoStack& undo_stack = m_app.scene().undo_stack();
  edit_menu.addAction(undo_stack.createUndoAction(nullptr));
  edit_menu.addAction(undo_stack.createRedoAction(nullptr));
}

MainMenuBar::MainMenuBar(Application& app, MainWindow& main_window)
  : m_app(app)
  , m_main_window(main_window)
{
  make_file_menu();
  make_edit_menu();
  make_create_menu();
  make_window_menu();
}

}  // namespace omm
