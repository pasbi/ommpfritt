#include "mainmenubar.h"

#include <memory>
#include <QAction>
#include <QMenu>

#include "application.h"

namespace
{

template<typename QObjectT, typename F>
void action(QMenu* menu, const QString& label, QObjectT* receiver, F f)
{
  omm::MainMenuBar::connect(menu->addAction(label), &QAction::triggered, receiver, f);
}

}  // namespace

namespace omm
{

void MainMenuBar::make_file_menu()
{
  auto file_menu = addMenu(tr("&File"));
  file_menu->setObjectName("my file menu");
  action(file_menu, tr("&new"), &m_app, &Application::new_project);
  action(file_menu, tr("&load"), &m_app, &Application::load);
  action(file_menu, tr("&save"), &m_app, static_cast<bool (Application::*)()>(&Application::save));
  action(file_menu, tr("&save_as"), &m_app, &Application::save_as);
}

omm::MainMenuBar::MainMenuBar(Application& app)
  : m_app(app)
{
  make_file_menu();
}

}  // namespace omm
