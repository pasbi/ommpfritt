#include "mainwindow.h"

#include <memory>
#include <glog/logging.h>

#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>

#include "mainwindow/viewport.h"
#include "mainwindow/mainmenubar.h"
#include "mainwindow/application.h"

#include "managers/propertymanager.h"
#include "managers/objectmanager/objectmanager.h"

namespace omm
{

MainWindow::MainWindow(Application& app)
{
  setMenuBar(std::make_unique<MainMenuBar>(app).release());

  setDockNestingEnabled(true);

  auto object_manager = std::make_unique<ObjectManager>(app.project().scene());
  object_manager->show();
  addDockWidget(Qt::RightDockWidgetArea, object_manager.release());

  auto property_manager = std::make_unique<PropertyManager>(app.project().scene());
  property_manager->show();
  addDockWidget(Qt::RightDockWidgetArea, property_manager.release());

  setCentralWidget(std::make_unique<Viewport>(app.project()).release());
}

}  // namespace omm