#include "path/graph.h"
#include "fmt/format.h"
#include "geometry/point.h"
#include "path/dedge.h"
#include "path/edge.h"
#include "path/face.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "path/pathvectorview.h"
#include "testutil.h"
#include "transform.h"
#include "gtest/gtest.h"
#include <QSize>

class TestCase : private ommtest::PathVectorHeap
{
public:
  TestCase(std::unique_ptr<omm::PathVector>&& path_vector, std::set<omm::PathVectorView>&& pvvs,
           const std::size_t n_expected_components, std::string name, const bool is_planar = true)
    : m_path_vector(annex(std::move(path_vector)))
    , m_expected_faces(util::transform<omm::Face>(std::move(pvvs)))
    , m_n_expected_components(n_expected_components)
    , m_name(std::move(name))
    , m_is_planar(is_planar)
  {
  }

  template<typename Edges> TestCase(std::unique_ptr<omm::PathVector>&& path_vector, std::set<Edges>&& edgess,
                                    const std::size_t n_expected_components, std::string name)
    : TestCase(std::move(path_vector), util::transform<omm::PathVectorView>(std::move(edgess)), n_expected_components,
               std::move(name), true)
  {
  }

  TestCase(std::unique_ptr<omm::PathVector>&& path_vector, const std::size_t n_expected_components, std::string name)
    : TestCase(std::move(path_vector), {}, n_expected_components, std::move(name), false)
  {
  }

  [[nodiscard]] const omm::PathVector& path_vector() const
  {
    return *m_path_vector;
  }

  [[nodiscard]] const std::set<omm::Face>& expected_faces() const
  {
    return m_expected_faces;
  }

  [[nodiscard]] bool is_planar() const
  {
    return m_is_planar;
  }

  [[nodiscard]] TestCase add_arm(const std::size_t path_index, const std::size_t point_index,
                                 std::vector<omm::Point> geometries) &&
  {
    const auto* const hinge = m_path_vector->paths().at(path_index)->points().at(point_index);
    auto& arm = m_path_vector->add_path();
    auto last_point = m_path_vector->share(*hinge);
    for (auto g : geometries) {
      g.set_position(g.position() + hinge->geometry().position());
      auto next_point = std::make_shared<omm::PathPoint>(g, m_path_vector);
      arm.add_edge(last_point, next_point);
      last_point = next_point;
    }
    m_name += fmt::format("-arm[{}.{}-{}]", path_index, point_index, geometries.size());
    return std::move(*this);
  }

  [[nodiscard]] TestCase add_loop(const std::size_t path_index, const std::size_t point_index, const double arg0,
                                  const double arg1) &&
  {
    const auto& src_path = *m_path_vector->paths().at(path_index);
    auto& loop = m_path_vector->add_path();
    auto* const hinge = src_path.points().at(point_index);
    hinge->geometry().set_tangent({&loop, omm::Direction::Forward}, omm::PolarCoordinates(arg0, 1.0));
    hinge->geometry().set_tangent({&loop, omm::Direction::Backward}, omm::PolarCoordinates(arg1, 1.0));
    auto shared_hinge = src_path.share(*hinge);
    auto& loop_edge = loop.add_edge(shared_hinge, shared_hinge);
    m_expected_faces.emplace(omm::PathVectorView({omm::DEdge::fwd(&loop_edge)}));

    static constexpr auto deg = M_1_PI * 180.0;
    m_name += fmt::format("-loop[{}.{}-{}]", path_index, point_index, arg0 * deg, arg1 * deg);
    return std::move(*this);
  }

  [[nodiscard]] TestCase add_loops(const std::size_t path_index, const std::size_t point_index, const double arg0,
                                   const double arg1, const std::size_t count) &&
  {
    auto tc = std::move(*this);
    const double advance = (arg1 - arg0) / static_cast<double>(2 * count);
    for (std::size_t i = 0; i < count; ++i) {
      const double start = arg0 + 2 * i * advance;
      const double end = arg0 + 2 * (i + 1) * advance;
      tc = std::move(tc).add_loop(path_index, point_index, start, end);
    }
    return tc;
  }

  [[nodiscard]] auto n_expected_components() const
  {
    return m_n_expected_components;
  }

  friend std::ostream& operator<<(std::ostream& os, const TestCase& tc)
  {
    return os << tc.m_name;
  }

private:
  omm::PathVector* m_path_vector;
  std::set<omm::Face> m_expected_faces;
  std::size_t m_n_expected_components;
  std::string m_name;
  bool m_is_planar;
};

[[nodiscard]] TestCase empty_paths(const std::size_t path_count)
{
  auto pv = std::make_unique<omm::PathVector>();
  for (std::size_t i = 0; i < path_count; ++i) {
    pv->add_path();
  }
  return {std::move(pv), {}, 0, fmt::format("{}-empty paths", path_count)};
}

[[nodiscard]] TestCase ellipse(ommtest::EllipseMaker ellipse_maker)
{
  auto pv = std::make_unique<omm::PathVector>();
  ellipse_maker.make_path(*pv);
  return {std::move(pv), ellipse_maker.faces(), 1, ellipse_maker.to_string()};
}

[[nodiscard]] TestCase rectangles(const std::size_t count)
{
  auto pv = std::make_unique<omm::PathVector>();
  std::set<std::deque<omm::DEdge>> expected_pvvs;

  for (std::size_t i = 0; i < count; ++i) {
    auto& path = pv->add_path();
    const auto p = [pv=pv.get()](const double x, const double y) {
      return std::make_shared<omm::PathPoint>(omm::Point({x, y}), pv);
    };

    const double x = i * 3.0;

    std::deque<omm::DEdge> edges;
    path.set_single_point(p(x - 1.0, -1.0));
    edges.emplace_back(&path.add_edge(path.last_point(), p(x + 1.0, -1.0)), omm::Direction::Forward);
    edges.emplace_back(&path.add_edge(path.last_point(), p(x + 1.0, 1.0)), omm::Direction::Forward);
    edges.emplace_back(&path.add_edge(path.last_point(), p(x - 1.0, 1.0)), omm::Direction::Forward);
    edges.emplace_back(&path.add_edge(path.last_point(), path.first_point()), omm::Direction::Forward);
    expected_pvvs.emplace(std::move(edges));
  }
  return {std::move(pv), std::move(expected_pvvs), count, fmt::format("{} Rectangles", count)};
}

[[nodiscard]] TestCase grid(const QSize& size, const QMargins& margins)
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

  std::set<std::deque<omm::DEdge>> expected_pvvs;
  if (!h_paths.empty() && !v_paths.empty()) {
    for (std::size_t x = 0; x < v_paths.size() - 1; ++x) {
      for (std::size_t y = 0; y < h_paths.size() - 1; ++y) {
        expected_pvvs.emplace(std::deque{omm::DEdge::fwd(h_paths.at(y + 0)->edges().at(x + margins.left())),
                                         omm::DEdge::fwd(v_paths.at(x + 1)->edges().at(y + margins.top())),
                                         omm::DEdge::fwd(h_paths.at(y + 1)->edges().at(x + margins.left())),
                                         omm::DEdge::fwd(v_paths.at(x + 0)->edges().at(y + margins.top()))});
      }
    }
  }

  const auto name = [m = margins, size]() {
    static constexpr auto fmt_m = [](const int value, const auto& name) {
      return value == 0 ? "" : fmt::format("-{}={}", name, value);
    };
    const auto s_ms = fmt_m(m.left(), "l") + fmt_m(m.right(), "r") + fmt_m(m.top(), "t") + fmt_m(m.bottom(), "b");
    return fmt::format("{}x{}-Grid{}", size.width(), size.height(), s_ms);
  };

  return {std::move(pv), std::move(expected_pvvs), 1, name()};
}

[[nodiscard]] TestCase leaf(std::vector<int> counts)
{
  assert(counts.size() >= 2);
  auto pv = std::make_unique<omm::PathVector>();
  auto start = std::make_shared<omm::PathPoint>(omm::Point({1.0, 0.0}), pv.get());
  auto end = std::make_shared<omm::PathPoint>(omm::Point({0.0, 1.0}), pv.get());
  for (std::size_t i = 0; i < counts.size(); ++i) {
    const double s = static_cast<double>(i) / (static_cast<double>(counts.size() - 1));
    auto& path = pv->add_path();
    auto last_point = start;
    for (int j = 0; j < counts.at(i); ++j) {
      const double arg = static_cast<double>(j + 1) / static_cast<double>(counts.at(i) + 1) * M_PI / 2.0;
      const omm::Vec2f p(std::cos(arg), std::sin(arg));
      const omm::Vec2f q(1.0 - std::sin(arg), 1.0 - std::cos(arg));
      const auto x = std::lerp(p.x, q.x, s);
      const auto y = std::lerp(p.y, q.y, s);
      auto current = std::make_shared<omm::PathPoint>(omm::Point({x, y}), pv.get());
      path.add_edge(last_point, current);
      last_point = current;
    }
    path.add_edge(last_point, end);
  }

  std::set<omm::PathVectorView> expected_pvvs;
  for (std::size_t i = 1; i < counts.size(); ++i) {
    const auto* const current = pv->paths()[i];
    const auto* const previous = pv->paths()[i - 1];
    std::deque<omm::DEdge> edges;
    for (auto* const e : current->edges()) {
      edges.emplace_back(omm::DEdge::fwd(e));
    }
    const auto previous_edges = previous->edges();
    for (auto it = previous_edges.rbegin(); it != previous_edges.rend(); ++it) {
      edges.emplace_back(omm::DEdge::bwd(*it));
    }
    expected_pvvs.emplace(edges);
  }

  static constexpr auto format_count = [](const auto accu, const auto c) { return accu + fmt::format("-{}", c); };
  const auto s_counts = std::accumulate(counts.begin(), counts.end(), std::string{}, format_count);

  return {std::move(pv), std::move(expected_pvvs), 1, "leaf" + s_counts};
}

[[nodiscard]] TestCase blossom(const std::vector<int>& segments, const double spacing)
{
  auto pv = std::make_unique<omm::PathVector>();
  auto center = std::make_shared<omm::PathPoint>(omm::Point(), pv.get());
  std::set<std::deque<omm::DEdge>> expected_pvvs;

  const auto add_petal =
      [&center, &pv = *pv](const double start_angle, const double end_angle, const int subdivisions) -> auto&
  {
    auto& path = pv.add_path();
    const auto tangent_length = 2.0;
    std::vector<std::shared_ptr<omm::PathPoint>> points;
    center->geometry().set_tangent({&path, omm::Direction::Forward},
                                   omm::PolarCoordinates(start_angle, tangent_length));
    center->geometry().set_tangent({&path, omm::Direction::Backward}, omm::PolarCoordinates(end_angle, tangent_length));
    if (subdivisions == 0) {
      path.add_edge(center, center);
    } else if (subdivisions == 1) {
      const auto phi = (start_angle + end_angle) / 2.0;
      const omm::PolarCoordinates intermediate_position(phi, tangent_length * 2.0);
      omm::Point intermediate_geometry(intermediate_position.to_cartesian());
      const omm::PolarCoordinates tangent(phi + M_PI_2, tangent_length * 0.7);
      intermediate_geometry.set_tangent({&path, omm::Direction::Forward}, tangent);
      intermediate_geometry.set_tangent({&path, omm::Direction::Backward}, -tangent);
      auto intermediate = std::make_shared<omm::PathPoint>(intermediate_geometry, &pv);
      path.add_edge(center, intermediate);
      path.add_edge(intermediate, center);
    } else {
      // More than one subdivision is not yet implemented.
      exit(1);
    }
    return path;
  };

  for (std::size_t i = 0; i < segments.size(); ++i) {
    const auto n = static_cast<double>(segments.size());
    const auto start_angle = static_cast<double>(i + spacing / 2.0) / n * 2.0 * M_PI;
    const auto end_angle = static_cast<double>(i + 1 - spacing / 2.0) / n * 2.0 * M_PI;
    auto& path = add_petal(start_angle, end_angle, segments.at(i));
    expected_pvvs.insert(
        util::transform<std::deque>(path.edges(), [](auto* const edge) { return omm::DEdge::fwd(edge); }));
  }

  const auto name =
      "blossom-"
      + static_cast<QStringList>(util::transform<QList>(segments, [](const int n) { return QString("%1").arg(n); }))
            .join("-")
      + QString("_%1").arg(spacing);
  return {std::move(pv), std::move(expected_pvvs), segments.size(), name.toStdString()};
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
  for (auto face : GetParam().expected_faces()) {
    const auto edges = face.path_vector_view().normalized();
    for (std::size_t i = 1; i < edges.size(); ++i) {
      ASSERT_LT(edges.front(), edges.at(i));
    }
    if (edges.size() > 2) {
      ASSERT_LT(edges.at(1), edges.back());
    }
  }
}

TEST_P(GraphTest, ComputeFaces)
{
  static constexpr auto print_graph_into_svg = false;

  const auto& test_case = GetParam();

  if constexpr (print_graph_into_svg) {
    ommtest::Application app;
    QString name(::testing::UnitTest::GetInstance()->current_test_case()->name());
    name.replace("/", "_");
    std::ostringstream oss;
    oss << "/tmp/foo_" << name.toStdString() << "_" << test_case << ".svg";
    const auto fname = QString::fromStdString(oss.str());
    test_case.path_vector().to_svg(fname);
    LDEBUG << "save svg file " << fname;
  }

  std::set<omm::Face> actual_faces;
  ASSERT_NO_THROW(actual_faces = test_case.path_vector().faces());
  if (test_case.is_planar()) {
    // computing faces only makes sense if the graph is planar.
    ASSERT_EQ(test_case.expected_faces(), actual_faces);
  }
}

TEST_P(GraphTest, ConnectedComponents)
{
  const auto& test_case = GetParam();
  EXPECT_EQ(omm::Graph(test_case.path_vector()).connected_components().size(), test_case.n_expected_components());
}

[[nodiscard]] std::vector<omm::Point> linear_arm_geometry(const std::size_t length, const omm::Vec2f& direction)
{
  std::vector<omm::Point> ps;
  ps.reserve(length);
  for (std::size_t i = 0; i < length; ++i) {
    ps.emplace_back(omm::Point(static_cast<double>(i + 1) * direction));
  }
  return ps;
}

[[nodiscard]] TestCase special_test(const std::size_t variant)
{
  using PC = omm::PolarCoordinates;
  using D = omm::Direction;
  auto pv = std::make_unique<omm::PathVector>();
  auto& outer = pv->add_path();
  const auto make_point = [&outer, &pv = *pv](const omm::Vec2f& pos) {
    omm::Point geometry{pos};
    geometry.set_tangent({&outer, omm::Direction::Backward}, PC{});
    geometry.set_tangent({&outer, omm::Direction::Forward}, PC{});
    return std::make_unique<omm::PathPoint>(geometry, &pv);
  };
  std::deque<std::shared_ptr<omm::PathPoint>> points;
  points.emplace_back(make_point({-75.5, -155.0}));
  points.emplace_back(make_point({198.5, -183.0}));
  points.emplace_back(make_point({281.5, -10.0}));
  points.emplace_back(make_point({237.5, 129.0}));
  points.emplace_back(make_point({-39.5, 83.0}));

  for (std::size_t i = 0; i < points.size(); ++i) {
    outer.add_edge(points.at(i), points.at((i + 1) % points.size()));
  }

  auto& inner = pv->add_path();
  inner.add_edge(points.at(1), points.at(4));

  std::deque face_1{
      omm::DEdge::fwd(outer.edges().at(1)),
      omm::DEdge::fwd(outer.edges().at(2)),
      omm::DEdge::fwd(outer.edges().at(3)),
      omm::DEdge::bwd(inner.edges().at(0)),
  };

  std::deque face_2{
      omm::DEdge::fwd(outer.edges().at(0)),
      omm::DEdge::fwd(inner.edges().at(0)),
      omm::DEdge::fwd(outer.edges().at(4)),
  };

  std::set expected_faces{face_1, face_2};
  switch (variant) {
  case 0:
    points.at(4)->geometry().set_tangent({&inner, D::Backward}, PC{});
    points.at(4)->geometry().set_tangent({&outer, D::Backward}, PC{1.0303768265243127, 99.12618221237013});
    points.at(4)->geometry().set_tangent({&outer, D::Forward}, PC{-2.1112158270654806, 99.12618221237013});
    break;
  case 1:
    points.at(4)->geometry().set_tangent({&inner, D::Backward}, PC{-0.6675554919511357, 250.7695451381673});
    points.at(4)->geometry().set_tangent({&outer, D::Backward}, PC{1.0303768265243127, 99.12618221237013});
    points.at(4)->geometry().set_tangent({&outer, D::Forward}, PC{-2.1112158270654806, 99.12618221237013});
    break;
  case 2:
    points.at(4)->geometry().set_tangent({&inner, D::Backward}, PC{-0.6675554919511357, 350.7695451381673});
    points.at(4)->geometry().set_tangent({&outer, D::Backward}, PC{1.0303768265243127, 99.12618221237013});
    points.at(4)->geometry().set_tangent({&outer, D::Forward}, PC{-2.1112158270654806, 99.12618221237013});
    expected_faces.clear();  // the graph is not planar
    break;
  case 3:
    points.at(1)->geometry().set_position({120.5, -143.0});
    points.at(1)->geometry().set_tangent({&inner, D::Backward}, PC{1.2350632478379595, 274.2411547737723});
    points.at(1)->geometry().set_tangent({&inner, D::Forward}, PC{0.09056247365766779, 0.0});
    points.at(1)->geometry().set_tangent({&inner, D::Backward}, PC{0.015592141134032511, 355.6724823321008});
    points.at(1)->geometry().set_tangent({&inner, D::Forward}, PC{-2.921946505938993, 570.3718878667855});
    break;
  default:
    assert(false);
  }

  const bool is_planar = variant != 2;
  if (is_planar) {
    return {std::move(pv), std::move(expected_faces), 1, "special-test-" + std::to_string(variant)};
  }
  return {std::move(pv), 1, "special-test-" + std::to_string(variant)};
}

// clang-format off
#define EXPAND_ELLIPSE(N, ext) \
  ellipse(ommtest::EllipseMaker{{0.0, 0.0}, {1.0, 1.0}, N, true, true}) ext, \
  ellipse(ommtest::EllipseMaker{{0.0, 0.0}, {1.0, 1.0}, N, false, true}) ext, \
  ellipse(ommtest::EllipseMaker{{0.0, 0.0}, {1.0, 1.0}, N, true, false}) ext, \
  ellipse(ommtest::EllipseMaker{{0.0, 0.0}, {1.0, 1.0}, N, false, false}) ext

const auto test_cases = ::testing::Values(
    empty_paths(0),
    empty_paths(1),
    empty_paths(10),
    rectangles(1),
    rectangles(3),
    rectangles(10),
    rectangles(1).add_arm(0, 0, linear_arm_geometry(3, {-1.0, 0.0})),
    rectangles(2).add_arm(1, 1, linear_arm_geometry(3, {-1.0, -1.0})),
    EXPAND_ELLIPSE(3, ),
    EXPAND_ELLIPSE(4, ),
    EXPAND_ELLIPSE(2, .add_arm(0, 1, linear_arm_geometry(3, {-1.0, -1.0}))),
    EXPAND_ELLIPSE(4, .add_arm(0, 1, linear_arm_geometry(3, {-1.0, -1.0}))),
    EXPAND_ELLIPSE(4, .add_loop(0, 2, M_PI - 1.0, M_PI + 1.0)),
    EXPAND_ELLIPSE(4, .add_loops(0, 2, M_PI - 1.0, M_PI + 1.0, 2)),
    EXPAND_ELLIPSE(4, .add_loops(0, 2, M_PI - 1.0, M_PI + 1.0, 6)),
    EXPAND_ELLIPSE(8, ),
    EXPAND_ELLIPSE(100, .add_arm(0, 2, linear_arm_geometry(2, {0.0, 1.0}))),
    grid({2, 3}, QMargins{}),
    grid({4, 4}, QMargins{}),
    grid({8, 7}, QMargins{1, 2, 2, 3}),
    leaf({2, 1, 5}),
    leaf({1, 3}),
    leaf({2, 1, 1, 2}),
    blossom({0, 0}, 0.5),
    blossom({0, 1}, 0.5),
    blossom({1, 1}, 0.5),
    blossom({0, 0, 0}, 0.0),
    blossom({0, 1, 0}, 0.0),
    blossom({1, 1, 1}, 0.0),
    blossom({0, 0, 0}, 0.1),
    blossom({0, 0, 0}, 0.9),
    blossom({0, 0, 0, 0, 0, 0, 0}, 0.0),
    blossom({0, 1, 1, 0, 1, 0, 0}, 0.0),
    special_test(0),
    special_test(1),
    special_test(2),
    special_test(3)
);
// clang-format on

INSTANTIATE_TEST_SUITE_P(P, GraphTest, test_cases);
