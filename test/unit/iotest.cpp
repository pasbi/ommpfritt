#include "config.h"
#include "logging.h"
#include "gtest/gtest.h"
#include "main/application.h"
#include "main/options.h"
#include "scene/scene.h"
#include "testutil.h"
#include <QApplication>

namespace
{


}  // namespace

TEST(IOTest, load_icons)
{
  static constexpr auto kill_after_msec = 1000;
  using namespace std::string_literals;
  auto app = ommtest::get_application(kill_after_msec);
  static const auto fn = QString::fromStdString(source_directory + "/icons/icons.omm"s);
  EXPECT_TRUE(app->scene->load_from(fn));
}
