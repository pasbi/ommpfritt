#include <glog/logging.h>

#include "application.h"
#include "ommpfrittconfig.h"

#include "mainwindow.h"

int main (int argc, char *argv[])
{
  google::InitGoogleLogging(argv[0]);

  Application app(argc, argv);
  MainWindow window;
  return app.run(window);
}