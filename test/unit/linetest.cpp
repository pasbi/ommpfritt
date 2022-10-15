#include "geometry/line.h"
#include "fmt/format.h"
#include "geometry/vec2.h"
#include "gtest/gtest.h"

class LineIntersectionTestCase
{
public:
  omm::Line a;
  omm::Line b;
  double expected_t;
  double expected_u;
  friend std::ostream& operator<<(std::ostream& os, const LineIntersectionTestCase& tc)
  {
    return os << tc.a << "-" << tc.b;
  }
};

class LineIntersectionTest : public ::testing::TestWithParam<LineIntersectionTestCase>
{
};

TEST_P(LineIntersectionTest, LineIntersection)
{
  static constexpr auto eps = 0.001;

  const auto tcase = GetParam();
  const auto t = tcase.a.intersect(tcase.b);
  const auto u = tcase.b.intersect(tcase.a);

  EXPECT_NEAR(tcase.a.lerp(t).x, tcase.b.lerp(u).x, eps);
  EXPECT_NEAR(tcase.a.lerp(t).y, tcase.b.lerp(u).y, eps);

  EXPECT_NEAR(t, tcase.expected_t, eps);
  EXPECT_NEAR(u, tcase.expected_u, eps);
}

// clang-format off

using LITC = LineIntersectionTestCase;
const auto litc_test_cases = ::testing::Values(
    LITC{{{0.0, 0.0}, {1.0, 1.0}}, {{0.0, 1.0}, {1.0, 0.0}}, 0.5, 0.5},
    LITC{{{0.0, 0.0}, {2.0, 2.0}}, {{0.0, 1.0}, {1.0, 0.0}}, 0.25, 0.5},
    LITC{{{0.0, 0.0}, {1.0, 0.0}}, {{0.5, 0.0}, {0.5, 1.0}}, 0.5, 0.0},
    LITC{{{0.0, 0.0}, {0.0, 1.0}}, {{-2.0, 0.3}, {6.0, 0.3}}, 0.3, 0.25}
);

// clang-format on

INSTANTIATE_TEST_SUITE_P(P, LineIntersectionTest, litc_test_cases);

class LineDistanceTestCase
{
public:
  omm::Line line;
  omm::Vec2f point;
  double expected_distance;
  friend std::ostream& operator<<(std::ostream& os, const LineDistanceTestCase& tc)
  {
    return os << tc.line << "-" << fmt::format("Point[{}, {}]", tc.point.x, tc.point.y);
  }
};

class LineDistanceTest : public ::testing::TestWithParam<LineDistanceTestCase>
{
};

TEST_P(LineDistanceTest, LineDistance)
{
  static constexpr auto eps = 0.001;

  const auto tcase = GetParam();
  EXPECT_NEAR(tcase.line.distance(tcase.point), tcase.expected_distance, eps);
  EXPECT_NEAR(tcase.line.distance(tcase.line.a), 0.0, eps);
  EXPECT_NEAR(tcase.line.distance(tcase.line.b), 0.0, eps);
}

// clang-format off

using LDTC = LineDistanceTestCase;
const auto ldtc_test_cases = ::testing::Values(
    LDTC{{{0.0, 0.0}, {1.0, 1.0}}, {0.0, 0.0}, 0.0},
    LDTC{{{0.0, 0.0}, {0.0, 2.0}}, {1.0, 10.0}, 1.0},
    LDTC{{{0.0, 0.0}, {2.0, 0.0}}, {-99.1234, -3}, 3.0},
    LDTC{{{0.0, 0.0}, {1.0, 1.0}}, {0.0, 1.0}, -std::sqrt(0.5)}
);

// clang-format on

INSTANTIATE_TEST_SUITE_P(P, LineDistanceTest, ldtc_test_cases);

class LineProjectionTestCase
{
public:
  omm::Line line;
  omm::Vec2f point;
  double expected_t;
  friend std::ostream& operator<<(std::ostream& os, const LineProjectionTestCase& tc)
  {
    return os << tc.line << "-" << fmt::format("Point[{}, {}]", tc.point.x, tc.point.y);
  }
};

class LineProjectionTest : public ::testing::TestWithParam<LineProjectionTestCase>
{
};

TEST_P(LineProjectionTest, LineProjection)
{
  static constexpr auto eps = 0.001;

  const auto tcase = GetParam();
  EXPECT_NEAR(tcase.line.project(tcase.point), tcase.expected_t, eps);
  EXPECT_NEAR(tcase.line.project(tcase.line.a), 0.0, eps);
  EXPECT_NEAR(tcase.line.project(tcase.line.b), 1.0, eps);
}

// clang-format off

using LPTC = LineProjectionTestCase;
const auto lptc_test_cases = ::testing::Values(
    LPTC{{{0.0, 0.0}, {1.0, 1.0}}, {1.0, 0.0}, 0.5},
    LPTC{{{0.0, 0.0}, {1.0, 1.0}}, {0.0, 0.3}, 0.15},
    LPTC{{{0.0, 0.0}, {0.0, 2.0}}, {3.1415, 0.468}, 0.234},
    LPTC{{{1.0, 2.0}, {3.0, 4.0}}, {5.0, 6.0}, 2.0}
);

// clang-format on

INSTANTIATE_TEST_SUITE_P(P, LineProjectionTest, lptc_test_cases);
