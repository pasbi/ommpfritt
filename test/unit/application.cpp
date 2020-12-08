#include "mainwindow/application.h"
#include "common.h"
#include "logging.h"
#include "mainwindow/options.h"
#include "ommpfrittconfig.h"
#include "tools/selectobjectstool.h"
#include "tools/toolbox.h"
#include "gtest/gtest.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMainWindow>
#include <map>

std::unique_ptr<omm::Application> get_application(int kill_after_msec = 1)
{
  std::vector<std::string> args{"omm"};
  char** argv = new char*[1];
  argv[0] = args[0].data();
  int argc = 1;
  QApplication app(argc, argv);

  auto options = std::make_unique<omm::Options>(false,  // is_cli
                                                false  // have_opengl
  );
  auto omm_app = std::make_unique<omm::Application>(app, std::move(options));

  QTimer::singleShot(kill_after_msec, &app, [] { QApplication::quit(); });
  return omm_app;
}

// This kind of test crashes my IDE (QtCreator).
// it runs find on command line, though.

// TEST(application, read)
//{
//  auto app = get_application();
//  EXPECT_FALSE(app->scene.load_from("x"));
//  EXPECT_TRUE(app->scene.load_from(source_directory +
//  QStringLiteral("/sample-scenes/basic.omm")));
//}
