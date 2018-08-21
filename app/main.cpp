#include <glog/logging.h>

#include "application.h"
#include "ommpfrittconfig.h"

#include "mainwindow.h"

int main (int argc, char *argv[])
{
  google::InitGoogleLogging(argv[0]);

  omm::Application app(argc, argv);
  omm::MainWindow window;
  return app.run(window);
}