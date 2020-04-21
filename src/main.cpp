#include "mainwindow/application.h"
#include "ommpfrittconfig.h"
#include "tools/selectobjectstool.h"
#include "tools/toolbox.h"
#include <QApplication>
#include <QTranslator>
#include "mainwindow/mainwindow.h"
#include <QSettings>
#include <QVariant>
#include <QDirIterator>
#include "logging.h"
#include "animation/track.h"
#include "qapplication.h"

int main (int argc, char *argv[])
{
  QApplication qt_app(argc, argv);
  omm::Application app(qt_app);

  omm::MainWindow window(app);
  app.set_main_window(window);
  window.show();

  if (argc > 1) {
    app.scene.load_from(argv[1]);
  }

  app.scene.tool_box().set_active_tool(omm::SelectObjectsTool::TYPE);

  return qt_app.exec();
}
