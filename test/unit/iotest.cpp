#include "logging.h"
#include "gtest/gtest.h"
#include "testconfig.h"
#include "main/application.h"
#include "main/options.h"
#include "scene/scene.h"
#include "testutil.h"
#include <QApplication>

namespace
{

std::unique_ptr<omm::Application> get_application(int kill_after_msec = 1)
{
  auto app = std::make_unique<ommtest::GuiApplication>();

  auto options = std::make_unique<omm::Options>(false, // is_cli
                                                false  // have_opengl
  );
  auto omm_app = std::make_unique<omm::Application>(app->gui_application(), std::move(options));

  QTimer::singleShot(kill_after_msec, omm_app.get(), [app=app.release()] {
    QApplication::quit();
    delete app;
  });
  return omm_app;
}

}  // namespace

TEST(IOTest, load_icons)
{
  static constexpr auto kill_after_msec = 1000;
  using namespace std::string_literals;
  auto app = get_application(kill_after_msec);
  static const auto fn = QString::fromStdString(ommtest::SOURCE_DIRECTORY + "/icons/icons.omm"s);
  EXPECT_TRUE(app->scene->load_from(fn));
}
