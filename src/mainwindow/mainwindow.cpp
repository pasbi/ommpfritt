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
  setMenuBar(std::make_unique<MainMenuBar>(app, *this).release());

  setDockNestingEnabled(true);

  setCentralWidget(std::make_unique<Viewport>(app.project()).release());
}

}  // namespace omm