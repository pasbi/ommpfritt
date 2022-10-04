#include "geometry/vec2.h"
#include "path/face.h"
#include "gtest/gtest.h"

struct TestCase
{
  std::vector<omm::Vec2f> polygon;
  std::vector<omm::Vec2f> expected_points_inside;
  std::vector<omm::Vec2f> expected_points_outside;
};

class PolygonContainsTest : public ::testing::TestWithParam<TestCase>
{
};

TEST_P(PolygonContainsTest, Contains)
{
  auto test_case = GetParam();

  // the corner points are expected to be in the polygon.
  test_case.expected_points_inside.insert(test_case.expected_points_inside.end(), test_case.polygon.begin(),
                                          test_case.polygon.end());

  for (const auto& corner : test_case.expected_points_inside) {
    EXPECT_TRUE(omm::Face::polygon_contains(test_case.polygon, corner));
  }

  for (const auto& corner : test_case.expected_points_outside) {
    EXPECT_FALSE(omm::Face::polygon_contains(test_case.polygon, corner));
  }
}

// clang-format off
const auto polygon_test_cases = ::testing::Values(
TestCase{{{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}},
         {{0.5, 0.5}, {1.0, 0.5}, {0.5, 1.0}, {0.5, 0.0}, {0.0, 0.5}},
         {{2.0, 2.0}, {1.0, 2.0}, {0.5, 2.0}, {0.5, -2.0}, {-2.0, 0.5}, {2.0, 0.0}} }
);

// clang-format on
