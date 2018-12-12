#include "mainwindow/mainwindow.h"

#include <memory>
#include <glog/logging.h>

#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>
#include <QSettings>
#include <QCloseEvent>

#include "mainwindow/viewport/viewport.h"
#include "mainwindow/mainmenubar.h"
#include "mainwindow/application.h"

#include "managers/propertymanager/propertymanager.h"
#include "managers/objectmanager/objectmanager.h"
#include "scene/scene.h"

namespace omm
{

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
    auto manager = Manager::make(type, m_app.scene());
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

void MainWindow::closeEvent(QCloseEvent *event)
{
  save_state();
  QMainWindow::closeEvent(event);
}

}  // namespace omm