#include "mainmenubar.h"

#include <memory>
#include <QAction>
#include <QMenu>
#include <QMainWindow>

#include "mainwindow/application.h"
#include "commands/addobjectcommand.h"
#include "managers/objectmanager/objectmanager.h"

namespace
{

template<typename QObjectT, typename F>
void action(QMenu* menu, const QString& label, QObjectT* receiver, F f)
{
  omm::MainMenuBar::connect(menu->addAction(label), &QAction::triggered, receiver, f);
}

template<typename F> void action(QMenu* menu, const QString& label, F f)
{
  omm::MainMenuBar::connect(menu->addAction(label), &QAction::triggered, f);
}

}  // namespace

namespace omm
{

void MainMenuBar::make_file_menu()
{
  auto file_menu = addMenu(tr("&File"));
  action(file_menu, tr("&new"), &m_app, &Application::new_project);
  action(file_menu, tr("&load"), &m_app, &Application::load);
  action(file_menu, tr("&save"), &m_app, static_cast<bool (Application::*)()>(&Application::save));
  action(file_menu, tr("&save_as"), &m_app, &Application::save_as);
}

void MainMenuBar::make_create_menu()
{
  auto create_menu = addMenu(tr("&Create"));
  action(create_menu, tr("&empty"), [this]() {
    m_app.submit<AddObjectCommand>(std::make_unique<Object>(m_app.project().scene()));
  });
}

void MainMenuBar::make_window_menu()
{
  auto window_menu = addMenu(tr("&Window"));

  action(window_menu, tr("&object manager"), this, &MainMenuBar::show_manager<ObjectManager>);
  action(window_menu, tr("&property manager"), this, &MainMenuBar::show_manager<PropertyManager>);

}

MainMenuBar::MainMenuBar(Application& app, MainWindow& main_window)
  : m_app(app)
  , m_main_window(main_window)
{
  make_file_menu();
  make_create_menu();
  make_window_menu();
}

template<typename ManagerT> void MainMenuBar::show_manager()
{
  auto manager = std::make_unique<ManagerT>(m_app.project().scene());
  manager->show();
  auto& ref = *manager;
  m_main_window.addDockWidget(Qt::TopDockWidgetArea, manager.release());
  ref.setFloating(true);
}

}  // namespace omm
