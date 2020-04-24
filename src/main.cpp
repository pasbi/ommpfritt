#include "mainwindow/application.h"
#include <iostream>
#include "tools/selectobjectstool.h"
#include "tools/toolbox.h"
#include <QApplication>
#include "mainwindow/mainwindow.h"
#include <QSettings>
#include <QVariant>
#include <QDirIterator>
#include "logging.h"
#include "qapplication.h"
#include "logging.h"

QString level = "warning";
QFile logfile;
bool print_long_message = true;

int main (int argc, char *argv[])
{
  QApplication qt_app(argc, argv);

  omm::setup_logfile(logfile);
  qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& ctx, const QString &msg) {
    omm::handle_log(logfile, level, print_long_message, type, ctx, msg);
  });

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
