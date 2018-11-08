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
  action(create_menu, tr("&empty"), [this]() {
    m_app.scene().submit<AddObjectCommand>(std::make_unique<Object>());
  });
}

void MainMenuBar::make_window_menu()
{
  auto& window_menu = *addMenu(tr("&Window"));
  for (auto& manager_creator : MainWindow::manager_creators) {
    const auto label = qApp->translate("Manager", manager_creator.first.c_str());
    action(window_menu, label, [this, &manager_creator]() {
      auto manager = manager_creator.second(m_app.scene());
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
