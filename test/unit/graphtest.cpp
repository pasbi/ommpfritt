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
#include <QSize>


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

  TestCase add_arm(const std::size_t path_index, const std::size_t point_index, std::vector<omm::Point> geometries) &&
  {
    LINFO << "altering " << m_path_vector;
    const auto* const hinge = m_path_vector->paths().at(path_index)->points().at(point_index);
    auto& arm = m_path_vector->add_path();
    auto last_point = m_path_vector->share(*hinge);
    for (std::size_t i = 0; i < geometries.size(); ++i) {
      auto next_point = std::make_shared<omm::PathPoint>(geometries.at(i), m_path_vector);
      arm.add_edge(std::make_unique<omm::Edge>(last_point, next_point, &arm));
      last_point = next_point;
    }
    return std::move(*this);
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
  for (std::size_t i = 0; i < point_count - 1; ++i) {
    auto new_point = std::make_shared<omm::PathPoint>(geometry(i), pv.get());
    edges.emplace_back(&path.add_edge(path.last_point(), std::move(new_point)));
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

TestCase grid(const QSize& size, const QMargins& margins)
{
  auto pv = std::make_unique<omm::PathVector>();
  std::vector<std::vector<std::shared_ptr<omm::PathPoint>>> points(size.height());
  for (int y = 0; y < size.height(); ++y) {
    auto& row = points.at(y);
    row.reserve(size.width());
    for (int x = 0; x < size.width(); ++x) {
      const omm::Point geom({static_cast<double>(x), static_cast<double>(y)});
      row.emplace_back(std::make_shared<omm::PathPoint>(geom, pv.get()));
    }
  }

  std::deque<const omm::Path*> h_paths;
  for (int y = margins.top(); y < size.height() - margins.bottom(); ++y) {
    auto& path = pv->add_path();
    for (int x = 1; x < size.width(); ++x) {
      path.add_edge(points.at(y).at(x - 1), points.at(y).at(x));
    }
    h_paths.emplace_back(&path);
  }

  std::deque<const omm::Path*> v_paths;
  for (int x = margins.left(); x < size.width() - margins.right(); ++x) {
    auto& path = pv->add_path();
    for (int y = 1; y < size.height(); ++y) {
      path.add_edge(points.at(y - 1).at(x), points.at(y).at(x));
    }
    v_paths.emplace_back(&path);
  }

  std::set<std::deque<omm::Edge*>> expected_pvvs;
  if (!h_paths.empty() && !v_paths.empty()) {
    for (std::size_t x = 0; x < v_paths.size() - 1; ++x) {
      for (std::size_t y = 0; y < h_paths.size() - 1; ++y) {
        expected_pvvs.emplace(std::deque{h_paths.at(y + 0)->edges().at(x + margins.left()),
                                         v_paths.at(x + 1)->edges().at(y + margins.top()),
                                         h_paths.at(y + 1)->edges().at(x + margins.left()),
                                         v_paths.at(x + 0)->edges().at(y + margins.top())});
      }
    }
  }

  return {std::move(pv), std::move(expected_pvvs)};
}

class GraphTest : public ::testing::TestWithParam<TestCase>
{
};

TEST_P(GraphTest, FaceEqualityIsReflexive)
{
  for (const auto& face : GetParam().expected_faces()) {
    ASSERT_EQ(face, face);
  }
}

TEST_P(GraphTest, FacesAreDistinct)
{
  const auto faces = GetParam().expected_faces();
  for (auto it1 = faces.begin(); it1 != faces.end(); ++it1) {
    for (auto it2 = faces.begin(); it2 != faces.end(); ++it2) {
      if (it1 != it2) {
        ASSERT_NE(*it1, *it2);
        ASSERT_NE(*it2, *it1);
      }
    }
  }
}

TEST_P(GraphTest, RotationReverseInvariance)
{
  for (bool reverse : {true, false}) {
    for (const auto& face : GetParam().expected_faces()) {
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

TEST_P(GraphTest, Normalization)
{
  const auto test_case = GetParam();
  LINFO << test_case.path_vector().to_dot();
  for (auto face : GetParam().expected_faces()) {
    face.normalize();
    const auto& edges = face.path_vector_view().edges();
    for (std::size_t i = 1; i < edges.size(); ++i) {
      ASSERT_LT(edges.front(), edges.at(i));
    }
    ASSERT_LT(edges.at(1), edges.back());
  }
}

TEST_P(GraphTest, ComputeFaces)
{
  const auto& test_case = GetParam();
//  std::cout << test_case.path_vector().to_dot().toStdString() << std::endl;
  const auto actual_faces = test_case.path_vector().faces();
//  for (auto& face : test_case.expected_faces()) {
//    std::cout << "E: " << face.to_string().toStdString() << std::endl;
//  }
//  for (auto& face : actual_faces) {
//    std::cout << "A: " << face.to_string().toStdString() << std::endl;
//  }
  ASSERT_EQ(test_case.expected_faces(), actual_faces);
}

std::vector<omm::Point> linear_arm_geometry(const std::size_t length, const omm::Vec2f& start, const omm::Vec2f& direction)
{
  std::vector<omm::Point> ps;
  ps.reserve(length);
  for (std::size_t i = 0; i < length; ++i) {
    ps.emplace_back(omm::Point(start + static_cast<double>(i) * direction));
  }
  return ps;
}

#define EXPAND_ELLIPSE(N, ext) \
ellipse(N, true, true)ext, \
ellipse(N, false, true)ext, \
ellipse(N, true, false)ext, \
ellipse(N, false, false)ext

const auto test_cases = ::testing::Values(
    empty_paths(0),
    empty_paths(1),
    empty_paths(10),
    rectangles(1),
    rectangles(3),
    rectangles(10),
    rectangles(10).add_arm(0, 0, linear_arm_geometry(0, {0.0, 0.0}, {-1.0, 0.0})),
    rectangles(10).add_arm(0, 0, linear_arm_geometry(2, {0.0, 0.0}, {-1.0, 0.0})),
    rectangles(10).add_arm(3, 1, linear_arm_geometry(2, {0.0, 0.0}, {0.0, 1.0})),
    EXPAND_ELLIPSE(3,),
    EXPAND_ELLIPSE(4,),
    EXPAND_ELLIPSE(4, .add_arm(0, 2, linear_arm_geometry(2, {0.0, 0.0}, {0.0, 1.0}))),
    EXPAND_ELLIPSE(100,),
    EXPAND_ELLIPSE(100, .add_arm(0, 2, linear_arm_geometry(2, {0.0, 0.0}, {0.0, 1.0}))),
    grid({2, 3}, QMargins{}),
    grid({4, 4}, QMargins{}),
    grid({8, 7}, QMargins{1, 2, 2, 3})
);

INSTANTIATE_TEST_SUITE_P(P, GraphTest, test_cases);
