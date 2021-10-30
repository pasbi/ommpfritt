#include "logging.h"
#include "gtest/gtest.h"
#include "testconfig.h"
#include "main/application.h"
#include "main/options.h"
#include "scene/scene.h"
#include <QApplication>

namespace
{

template<std::size_t N>
std::vector<char*> string_array_to_charpp(std::array<std::string, N>& string_array)
{
  std::vector<char*> vs;
  vs.reserve(N);
  for (std::size_t i = 0; i < N; ++i) {
    vs.push_back(string_array.at(i).data());
  }
  return vs;
}

std::unique_ptr<omm::Application> get_application(int kill_after_msec = 1)
{
  std::array<std::string, 3> args{"omm", "-platform", "offscreen"};
  int argc = args.size();
  auto argv = string_array_to_charpp(args);
  QApplication app(argc, argv.data());

  auto options = std::make_unique<omm::Options>(false, // is_cli
                                                false  // have_opengl
  );
  auto omm_app = std::make_unique<omm::Application>(app, std::move(options));

  QTimer::singleShot(kill_after_msec, &app, [] { QApplication::quit(); });
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
