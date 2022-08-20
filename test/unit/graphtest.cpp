#include "gtest/gtest.h"
#include "path/face.h"
#include "path/graph.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathvector.h"
#include "geometry/point.h"
#include "path/pathpoint.h"
#include "path/pathvectorview.h"
#include "transform.h"

#include <path/pathvectorview.h>

omm::Face make_face(std::deque<omm::Edge*> edges)
{
  return omm::Face(omm::PathVectorView(std::move(edges)));
}

class TestCase
{
public:
  TestCase(omm::PathVector&& path_vector, std::set<std::deque<omm::Edge*>>&& edgess)
      : m_expected_faces(util::transform(edgess, make_face))
  {
    m_path_vectors.push_back(std::move(path_vector));
    m_path_vector = &m_path_vectors.back();
  }

  const omm::PathVector& path_vector() const
  {
    return *m_path_vector;
  }

  const std::set<omm::Face>& expected_faces() const
  {
    return m_expected_faces;
  }

private:
  omm::PathVector* m_path_vector;
  std::set<omm::Face> m_expected_faces;

private:
  // The TestCase objects are not persistent, hence the path vectors need to be stored beyond the
  // lifetime of the test case.
  static std::list<omm::PathVector> m_path_vectors;
};

std::list<omm::PathVector> TestCase::m_path_vectors;

TestCase empty_paths(const std::size_t path_count)
{
  omm::PathVector pv;
  for (std::size_t i = 0; i < path_count; ++i) {
    pv.add_path();
  }
  return {std::move(pv), {}};
}

//TestCase ellipse(const std::size_t point_count, const bool closed)
//{
//  const auto geometry = [point_count](const std::size_t i) {
//      const double theta = M_PI * 2.0 * static_cast<double>(i) / static_cast<double>(point_count);
//      const omm::Vec2f pos(std::cos(theta), std::sin(theta));
//      const omm::Vec2f t(-std::sin(theta), std::cos(theta));
//      return omm::Point(pos, omm::PolarCoordinates(t), omm::PolarCoordinates(-t));
//  };

//  omm::PathVector pv;
//  auto& path = pv.add_path();
//  if (point_count == 1) {
//    path.set_single_point(std::make_shared<omm::PathPoint>(geometry(0), &pv));
//  } else if (point_count > 1) {
//    for (std::size_t i = 0; i < point_count; ++i) {
//      path.add_edge(path.last_point(), std::make_shared<omm::PathPoint>(geometry(i), &pv));
//    }
//    if (closed) {
//      path.add_edge(path.last_point(), path.first_point());
//    }
//  }
//  return {pv, {}};
//}

TestCase rectangle()
{
  omm::PathVector pv;
  auto& path = pv.add_path();
  const auto p = [pv=&pv](const double x, const double y) {
    return std::make_shared<omm::PathPoint>(omm::Point({x, y}), pv);
  };

  std::deque<omm::Edge*> edges;
  path.set_single_point(p(-1, -1));
  edges.emplace_back(&path.add_edge(path.last_point(), p(1, -1)));
  edges.emplace_back(&path.add_edge(path.last_point(), p(1, 1)));
  edges.emplace_back(&path.add_edge(path.last_point(), p(-1, 1)));
  edges.emplace_back(&path.add_edge(path.last_point(), path.first_point()));
  return {std::move(pv), std::set{std::move(edges)}};
}

class GraphTest : public ::testing::TestWithParam<TestCase>
{
};

TEST_P(GraphTest, ComputeFaces)
{
  const auto& test_case = GetParam();
  omm::Graph graph(test_case.path_vector());
  const auto actual_faces = graph.compute_faces();
//  std::cout << graph.to_dot().toStdString() << std::endl;
  for (auto& face : test_case.expected_faces()) {
    std::cout << "E: " << face.to_string().toStdString() << std::endl;
  }
  for (auto& face : actual_faces) {
    std::cout << "A: " << face.to_string().toStdString() << std::endl;
  }
  ASSERT_EQ(test_case.expected_faces(), actual_faces);
}

INSTANTIATE_TEST_SUITE_P(
    EmptyPaths,
    GraphTest,
    ::testing::Values(
        empty_paths(0),
        empty_paths(1),
        empty_paths(10)
        ));

INSTANTIATE_TEST_SUITE_P(
    SingleFace,
    GraphTest,
    ::testing::Values(
        rectangle()
// add faces with `n` points
        ));
