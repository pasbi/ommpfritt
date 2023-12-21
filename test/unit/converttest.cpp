#include "common.h"
#include "config.h"
#include <nlohmann/json.hpp>
#include "gtest/gtest.h"
#include "main/application.h"
#include "main/options.h"
#include "mainwindow/pathactions.h"
#include "objects/ellipse.h"
#include "objects/pathobject.h"
#include "scene/history/historymodel.h"
#include "path/pathvector.h"
#include "path/path.h"
#include "scene/scene.h"
#include "scene/disjointpathpointsetforest.h"
#include "testutil.h"

namespace
{

std::unique_ptr<omm::Options> options()
{
  return std::make_unique<omm::Options>(false, // is_cli
                                        false  // have_opengl
  );
}

}  // namespace

TEST(convert, ellipse)
{
  ommtest::Application test_app(::options());
  auto& app = test_app.omm_app();

  auto& e = app.insert_object(omm::Ellipse::TYPE, omm::Application::InsertionMode::Default);
  static constexpr auto corner_count = 12;
  e.property(omm::Ellipse::CORNER_COUNT_PROPERTY_KEY)->set(corner_count);
  app.scene->set_selection({&e});
  ASSERT_EQ(omm::path_actions::convert_objects(app).size(), 1);
  app.scene->history().undo();
  const auto cs = omm::path_actions::convert_objects(app);
  ASSERT_EQ(cs.size(), 1);
  auto* const po = ::type_cast<omm::PathObject*>(*cs.begin());
  ASSERT_NE(po, nullptr);
  auto& path_vector = po->geometry();
  ASSERT_EQ(path_vector.paths().size(), 1);
  const auto& path = *path_vector.paths().front();
  ASSERT_EQ(path.points().size(), corner_count + 1);
  EXPECT_TRUE(path_vector.joined_points_shared());
  const auto& joined_points = path_vector.joined_points();
  ASSERT_EQ(joined_points.sets().size(), 1);
  const auto set = joined_points.get(path.points().front());
  EXPECT_EQ(set, (std::set<omm::PathPoint*, std::less<>>{path.points().front(), path.points().back()}));
}
