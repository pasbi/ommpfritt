#include "gtest/gtest.h"
#include "logging.h"
#include "common.h"
#include <QApplication>
#include <map>
#include "mainwindow/options.h"
#include "tools/toolbox.h"
#include "tools/selectobjectstool.h"
#include <QMainWindow>
#include "mainwindow/application.h"
#include <QFile>
#include <QDateTime>
#include "ommpfrittconfig.h"
#include <QDir>


std::unique_ptr<omm::Application> get_application(int kill_after_msec = 1)
{
  std::vector<std::string> args { "omm" };
  char** argv = new char*[1];
  argv[0] = args[0].data();
  int argc = 1;
  QApplication app(argc, argv);

  auto options = std::make_unique<omm::Options>(
        false,    // is_cli
        false     // have_opengl
  );
  auto omm_app = std::make_unique<omm::Application>(app, std::move(options));

  QTimer::singleShot(kill_after_msec, [&app]() {
    app.quit();
  });
  return omm_app;
}

// This kind of test crashes my IDE (QtCreator).
// it runs find on command line, though.

//TEST(application, read)
//{
//  auto app = get_application();
//  EXPECT_FALSE(app->scene.load_from("x"));
//  EXPECT_TRUE(app->scene.load_from(source_directory + QStringLiteral("/sample-scenes/basic.omm")));
//}
