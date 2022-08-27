#include "gtest/gtest.h"
#include "path/face.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathvector.h"
#include "geometry/point.h"
#include "path/pathpoint.h"
#include "path/pathvectorview.h"
#include "transform.h"
#include <path/pathvectorview.h>


class TestCase
{
public:
  TestCase(std::unique_ptr<omm::PathVector>&& path_vector, std::set<omm::PathVectorView>&& pvvs)
      : m_path_vector(m_path_vectors.emplace_back(std::move(path_vector)).get())
      , m_expected_faces(util::transform<omm::Face>(std::move(pvvs)))
  {
  }

  template<typename Edges>
  TestCase(std::unique_ptr<omm::PathVector>&& path_vector, std::set<Edges>&& edgess)
      : TestCase(std::move(path_vector), util::transform<omm::PathVectorView>(std::move(edgess)))
  {
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
  static std::list<std::unique_ptr<omm::PathVector>> m_path_vectors;
};

std::list<std::unique_ptr<omm::PathVector>> TestCase::m_path_vectors;

TestCase empty_paths(const std::size_t path_count)
{
  auto pv = std::make_unique<omm::PathVector>();
  for (std::size_t i = 0; i < path_count; ++i) {
    pv->add_path();
  }
  return {std::move(pv), {}};
}

TestCase ellipse(const std::size_t point_count, const bool closed, const bool no_tangents)
{
  const auto geometry = [point_count, no_tangents](const std::size_t i) {
      const double theta = M_PI * 2.0 * static_cast<double>(i) / static_cast<double>(point_count);
      const omm::Vec2f pos(std::cos(theta), std::sin(theta));
      if (no_tangents) {
        return omm::Point(pos);
      } else {
        const omm::Vec2f t(-std::sin(theta), std::cos(theta));
        return omm::Point(pos, omm::PolarCoordinates(t), omm::PolarCoordinates(-t));
      }
  };

  auto pv = std::make_unique<omm::PathVector>();
  auto& path = pv->add_path();
  std::deque<omm::Edge*> edges;
  path.set_single_point(std::make_shared<omm::PathPoint>(geometry(0), pv.get()));
  for (std::size_t i = 0; i < point_count; ++i) {
    path.add_edge(path.last_point(), std::make_shared<omm::PathPoint>(geometry(i), pv.get()));
  }
  if (closed && point_count > 1) {
    edges.emplace_back(&path.add_edge(path.last_point(), path.first_point()));
    return {std::move(pv), std::set{std::move(edges)}};
  }
  return {std::move(pv), {}};
}

TestCase rectangles(const std::size_t count)
{
  auto pv = std::make_unique<omm::PathVector>();
  std::set<std::deque<omm::Edge*>> expected_pvvs;
  for (std::size_t i = 0; i < count; ++i) {
    auto& path = pv->add_path();
    const auto p = [pv=pv.get()](const double x, const double y) {
      return std::make_shared<omm::PathPoint>(omm::Point({x, y}), pv);
    };

    const double x = i * 3.0;

    std::deque<omm::Edge*> edges;
    path.set_single_point(p(x - 1.0, -1.0));
    edges.emplace_back(&path.add_edge(path.last_point(), p(x + 1.0, -1.0)));
    edges.emplace_back(&path.add_edge(path.last_point(), p(x + 1.0, 1.0)));
    edges.emplace_back(&path.add_edge(path.last_point(), p(x - 1.0, 1.0)));
    edges.emplace_back(&path.add_edge(path.last_point(), path.first_point()));
    expected_pvvs.emplace(std::move(edges));
  }
  return {std::move(pv), std::move(expected_pvvs)};
}

class FaceTest : public ::testing::TestWithParam<TestCase>
{
public:
  FaceTest()
    : faces(GetParam().expected_faces())
  {
  }

  const std::set<omm::Face> faces;
};

TEST_P(FaceTest, FaceEqualityIsReflexive)
{
  for (const auto& face : faces) {
    ASSERT_EQ(face, face);
  }
}

TEST_P(FaceTest, FacesAreDistinct)
{
  for (auto it1 = faces.begin(); it1 != faces.end(); ++it1) {
    for (auto it2 = faces.begin(); it2 != faces.end(); ++it2) {
      if (it1 != it2) {
        ASSERT_NE(*it1, *it2);
        ASSERT_NE(*it2, *it1);
      }
    }
  }
}

TEST_P(FaceTest, RotationReverseInvariance)
{
  for (bool reverse : {true, false}) {
    for (const auto& face : faces) {
      const auto edges = face.path_vector_view().edges();
      for (std::size_t i = 0; i < edges.size(); ++i) {
        auto rotated_edges = edges;
        std::rotate(rotated_edges.begin(), std::next(rotated_edges.begin(), i), rotated_edges.end());
        if (reverse) {
          std::reverse(rotated_edges.begin(), rotated_edges.end());
        }
        const omm::Face rotated_face{omm::PathVectorView(rotated_edges)};
        ASSERT_EQ(face, rotated_face);
        ASSERT_EQ(rotated_face, face);
      }
    }
  }
}

class GraphTest : public ::testing::TestWithParam<TestCase>
{
};

TEST_P(GraphTest, ComputeFaces)
{
  const auto& test_case = GetParam();
  const auto actual_faces = test_case.path_vector().faces();
  for (auto& face : test_case.expected_faces()) {
    std::cout << "E: " << face.to_string().toStdString() << std::endl;
  }
  for (auto& face : actual_faces) {
    std::cout << "A: " << face.to_string().toStdString() << std::endl;
  }
  ASSERT_EQ(test_case.expected_faces(), actual_faces);
}

const auto test_cases = ::testing::Values(
        empty_paths(0),
        empty_paths(0),
        empty_paths(1),
        empty_paths(10),
        rectangles(1),
        rectangles(2),
        rectangles(10),
        ellipse(3, true, true),
        ellipse(3, false, true),
        ellipse(4, false, true),
        ellipse(4, true, true),
        ellipse(100, false, true),
        ellipse(100, true, true)
    );

INSTANTIATE_TEST_SUITE_P(P, GraphTest, test_cases);
INSTANTIATE_TEST_SUITE_P(P, FaceTest, test_cases);
