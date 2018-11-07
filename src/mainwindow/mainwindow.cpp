#include "mainwindow/mainwindow.h"

#include <memory>
#include <glog/logging.h>

#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>
#include <QSettings>
#include <QCloseEvent>

#include "mainwindow/viewport.h"
#include "mainwindow/mainmenubar.h"
#include "mainwindow/application.h"

#include "managers/propertymanager/propertymanager.h"
#include "managers/objectmanager/objectmanager.h"

namespace
{

template<typename ManagerT> std::unique_ptr<omm::Manager> make_manager(omm::Scene& scene)
{
  return std::make_unique<ManagerT>(scene);
}

template<typename Manager1> omm::MainWindow::creator_map register_managers()
{
  omm::MainWindow::creator_map map;
  auto value = std::make_pair( std::string(Manager1::TYPE()),
                               &make_manager<Manager1>);
  map.insert(value);
  return map;
}

template<typename Manager1, typename Manager2, typename... Managers>
omm::MainWindow::creator_map register_managers()
{
  auto map = register_managers<Manager2, Managers...>();
  auto single_map = register_managers<Manager1>();
  map.insert(single_map.begin(), single_map.end());
  return map;
}

}  // namespace

namespace omm
{

const MainWindow::creator_map MainWindow::manager_creators = register_managers<
  ObjectManager, PropertyManager >();

MainWindow::MainWindow(Application& app)
  : m_app(app)
{
  setMenuBar(std::make_unique<MainMenuBar>(app, *this).release());

  setDockNestingEnabled(true);

  setCentralWidget(std::make_unique<Viewport>(app.scene()).release());

  restore_state();
}

MainWindow::~MainWindow()
{
}

void MainWindow::restore_state()
{
  LOG(INFO) << "restore-state";
  QSettings settings;
  restoreGeometry(settings.value("mainwindow/geometry").toByteArray());
  restoreState(settings.value("mainwindow/window_state").toByteArray());

  auto size = settings.beginReadArray("mainwindow/managers");
  for (decltype(size) i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    const auto type = settings.value("type").toString().toStdString();
    auto manager = make_manager(type);
    if (!restoreDockWidget(manager.release())) {
      LOG(WARNING) << "Failed to restore geometry of manager.";
    }
  }
  settings.endArray();

}

void MainWindow::save_state()
{
  QSettings settings;
  settings.setValue("mainwindow/geometry", saveGeometry());
  settings.setValue("mainwindow/window_state", saveState());

  settings.beginWriteArray("mainwindow/managers");
  {
    size_t i = 0;
    for (auto manager : findChildren<Manager*>()) {
      settings.setArrayIndex(i);
      LOG(INFO) << "save " << manager->type();
      settings.setValue("type", QString::fromStdString(manager->type()));
      i += 1;
    }
  }
  settings.endArray();
}

std::unique_ptr<Manager> MainWindow::make_manager(const std::string& type)
{
  const auto it = manager_creators.find(type);
  assert(it != manager_creators.end());
  return it->second(m_app.scene());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  save_state();
  QMainWindow::closeEvent(event);
}

}  // namespace omm