#include "commands/cutpathcommand.h"
#include "objects/pathobject.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "gtest/gtest.h"
#include <fmt/core.h>

struct KnifeTestCase
{
  omm::PathVector geometry_before;
  omm::PathVector geometry_after;
  std::vector<Geom::PathVectorTime> cuts;
  std::string name;
};

std::ostream& operator<<(std::ostream& os, const KnifeTestCase& test_case)
{
  return os << test_case.name;
}

KnifeTestCase cut_straight_line(const std::size_t point_count, const double t)
{
  static constexpr auto geometry = [](const double t) { return omm::Point({0.0, t}); };
  static constexpr auto generate_points = [](const std::size_t point_count, omm::PathVector& path_vector) {
    std::vector<std::shared_ptr<omm::PathPoint>> points;
    points.reserve(point_count);
    for (std::size_t i = 0; i < point_count; ++i) {
      const auto t = static_cast<double>(i) / (static_cast<double>(point_count) - 1.0);
      points.emplace_back(std::make_shared<omm::PathPoint>(geometry(t), &path_vector));
    }
    return points;
  };

  KnifeTestCase test_case;
  const auto points_before = generate_points(point_count, test_case.geometry_before);
  const auto points_after = generate_points(point_count, test_case.geometry_after);

  auto& path_before = test_case.geometry_before.add_path();
  auto& path_after = test_case.geometry_after.add_path();

  for (std::size_t i = 1; i < point_count; ++i) {
    path_before.add_edge(points_before.at(i - 1), points_before.at(i));

    const double lower = static_cast<double>(i - 1) / static_cast<double>(point_count);
    const double upper = static_cast<double>(i) / static_cast<double>(point_count);

    if (lower < t && t <= upper) {
      auto extra = std::make_shared<omm::PathPoint>(geometry(t), &test_case.geometry_after);
      path_after.add_edge(points_after.at(i - 1), extra);
      path_after.add_edge(extra, points_after.at(i));
    } else {
      path_after.add_edge(points_after.at(i - 1), points_after.at(i));
    }
  }

  test_case.cuts = {};  // TODO
  test_case.name = fmt::format("Straight line with {} points, cut at {}", point_count, t);
  return test_case;
}

class KnifeTest : public ::testing::TestWithParam<KnifeTestCase>
{
};

TEST_P(KnifeTest, KnifeCommand)
{
  const auto& test_case = GetParam();
  omm::PathObject path_object(nullptr, test_case.geometry_before);
  const auto path_object_geometry = std::make_unique<omm::PathVector>(*path_object.compute_geometry());
  omm::CutPathCommand cut_command(path_object, test_case.cuts);

  cut_command.redo();

  ASSERT_EQ(path_object.geometry(), test_case.geometry_after);

  cut_command.undo();

  EXPECT_EQ(path_object.geometry(), *path_object_geometry);
}

INSTANTIATE_TEST_SUITE_P(KnifeCommandTests, KnifeTest,
                         ::testing::ValuesIn(std::vector{
                             cut_straight_line(10, 0.5),
                         }));
