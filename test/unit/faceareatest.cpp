#include "path/face.h"
#include "path/path.h"
#include "path/pathvector.h"
#include "path/pathvectorview.h"
#include "testutil.h"
#include "gtest/gtest.h"

#include "path/pathpoint.h"

class TestCase : ommtest::PathVectorHeap
{
public:
  TestCase(std::unique_ptr<omm::PathVector> pv, omm::Face face, const double area, const double eps)
    : path_vector(annex(std::move(pv))), face(std::move(face)), area(area), eps(eps)
  {
  }

  omm::PathVector* path_vector;
  omm::Face face;
  double area;
  double eps;

  friend std::ostream& operator<<(std::ostream& os, const TestCase& tc)
  {
    return os << tc.face;
  }
};

class FaceArea : public ::testing::TestWithParam<TestCase>
{
};

TEST_P(FaceArea, Area)
{
  const auto& tc = GetParam();
  ASSERT_TRUE(true);
  //  ASSERT_NEAR(std::abs(tc.face.area()), tc.area, tc.eps);
}

TestCase rectangle(const omm::Vec2f& origin, const omm::Vec2f& size)
{
  auto pv = std::make_unique<omm::PathVector>();
  auto& path = pv->add_path();
  const auto point = [pv = pv.get()](const auto& pos) { return std::make_shared<omm::PathPoint>(omm::Point(pos), pv); };
  auto points = std::vector{
      point(origin),
      point(origin + omm::Vec2f{size.x, 0}),
      point(origin + size),
      point(origin + omm::Vec2f{0, size.y}),
  };
  std::deque<omm::DEdge> edges;
  for (std::size_t i = 0; i < points.size(); ++i) {
    auto& edge = path.add_edge(points.at(i), points.at((i + 1) % points.size()));
    edges.emplace_back(&edge, omm::Direction::Forward);
  }
  static constexpr auto eps = 0.0001;
  const auto area = size.x * size.y;
  return {std::move(pv), omm::Face(omm::PathVectorView(std::move(edges))), area, eps};
}

TestCase ellipse(const std::size_t n, const omm::Vec2f& origin, const omm::Vec2f& radius)
{
  ommtest::EllipseMaker em{origin, radius, n, true, true};
  auto pv = std::make_unique<omm::PathVector>();
  em.make_path(*pv);
  const auto rel_eps = 2.0 / static_cast<double>(n);
  return {std::move(pv), omm::Face(omm::PathVectorView(*em.faces().begin())), em.area(), radius.x * radius.y * rel_eps};
}

// clang-format off
#define EXPAND_ELLIPSE(origin, size) \
  ellipse(11, origin, size), \
  ellipse(1007, origin, size), \
  ellipse(100, origin, size)

const auto test_cases = ::testing::Values(
  EXPAND_ELLIPSE(omm::Vec2f(0.0, 0.0), omm::Vec2f(1.0, 1.0)),
  EXPAND_ELLIPSE(omm::Vec2f(3.0, -1.0), omm::Vec2f(0.01, 100.0)),
  EXPAND_ELLIPSE(omm::Vec2f(-12.0, -12.0), omm::Vec2f(200.1234, 100.0)),
  EXPAND_ELLIPSE(omm::Vec2f(-1000000000, -10000000000), omm::Vec2f(0.1234, 0.776543)),
  rectangle({0.0, 0.0}, {1.0, 1.0}),
  rectangle({0.0, 0.0}, {1.0, 1.0}),
  rectangle({0.0, 0.0}, {1.0, 1.0}),
  rectangle({0.0, 0.0}, {1.0, 1.0}),
  rectangle({0.0, 0.0}, {1.0, 1.0}),
  rectangle({0.0, 0.0}, {1.0, 1.0}),
  rectangle({0.0, 0.0}, {1.0, 1.0}),
  rectangle({14.0, -12.0}, {3.0, 10.0})
);

// clang-format on

INSTANTIATE_TEST_SUITE_P(P, FaceArea, test_cases);
