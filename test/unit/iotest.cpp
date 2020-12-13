#include "logging.h"
#include "gtest/gtest.h"
#include "testconfig.h"
#include "mainwindow/application.h"
#include <QApplication>

std::unique_ptr<omm::Application> get_application(int kill_after_msec = 1)
{
  std::vector<std::string> args{"omm"};
  char** argv = new char*[1];
  argv[0] = args[0].data();
  int argc = 1;
  QApplication app(argc, argv);

  auto options = std::make_unique<omm::Options>(false, // is_cli
                                                false  // have_opengl
  );
  auto omm_app = std::make_unique<omm::Application>(app, std::move(options));

  QTimer::singleShot(kill_after_msec, &app, [] { QApplication::quit(); });
  return omm_app;
}

TEST(IOTest, load_icons)
{
  using namespace std::string_literals;
  auto app = get_application(1000);
  static const auto fn = QString::fromStdString(ommtest::SOURCE_DIRECTORY + "/icons/icons.omm"s);
  EXPECT_TRUE(app->scene.load_from(fn));
}
