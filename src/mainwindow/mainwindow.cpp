#include "mainwindow.h"

#include <memory>
#include <glog/logging.h>

#include <QMenu>
#include <QMenuBar>

#include "mainwindow/mainmenubar.h"

namespace omm
{

MainWindow::MainWindow(Application& app)
{
  QMenuBar* mb = new MainMenuBar(app); //std::make_unique<MainMenuBar>(app).release();
  setMenuBar(mb);

  // // MainMenu

  // for (auto menu : mb->children()) {
  //   LOG(INFO) << menu->objectName().toStdString();
  //   for (auto action : menu->children()) {
  //     LOG(INFO) << "  " << action->objectName().toStdString();
  //   }
  // }

  // LOG(INFO) << "xxx " << mb->objectName().toStdString();
}

}  // namespace omm