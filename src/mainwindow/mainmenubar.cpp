#include "mainmenubar.h"

#include <memory>
#include <QAction>
#include <QMenu>

#include "mainwindow/application.h"
#include "commands/addobjectcommand.h"

namespace
{

template<typename QObjectT, typename F>
void action(QMenu* menu, const QString& label, QObjectT* receiver, F f)
{
  omm::MainMenuBar::connect(menu->addAction(label), &QAction::triggered, receiver, f);
}

template<typename F>
void action(QMenu* menu, const QString& label, F f)
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
  auto file_menu = addMenu(tr("&Create"));
  action(file_menu, tr("&empty"), [this]() {
    m_app.submit<AddObjectCommand>(std::make_unique<Object>(m_app.project().scene()));
  });
}

omm::MainMenuBar::MainMenuBar(Application& app)
  : m_app(app)
{
  make_file_menu();
  make_create_menu();
}

}  // namespace omm
