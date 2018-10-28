#include <glog/logging.h>

#include "mainwindow/application.h"
#include "ommpfrittconfig.h"
#include <QApplication>

#include "mainwindow/mainwindow.h"

int main (int argc, char *argv[])
{
  google::InitGoogleLogging(argv[0]);
  QApplication qt_app(argc, argv);
  omm::Application app(qt_app);

  omm::MainWindow window(app);
  app.set_main_window(window);
  window.show();

  return qt_app.exec();
}