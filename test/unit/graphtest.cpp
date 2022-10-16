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

class TestCase
{
public:
  TestCase(std::unique_ptr<omm::PathVector>&& path_vector, std::set<omm::PathVectorView>&& pvvs,
           const std::size_t n_expected_components, std::string name)
    : m_path_vector(m_path_vectors.emplace_back(std::move(path_vector)).get())
    , m_expected_faces(util::transform<omm::Face>(std::move(pvvs)))
    , m_n_expected_components(n_expected_components)
    , m_name(std::move(name))
  {
  }

  template<typename Edges> TestCase(std::unique_ptr<omm::PathVector>&& path_vector, std::set<Edges>&& edgess,
                                    const std::size_t n_expected_components, std::string name)
    : TestCase(std::move(path_vector), util::transform<omm::PathVectorView>(std::move(edgess)), n_expected_components,
               std::move(name))
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

  TestCase add_loop(const std::size_t path_index, const std::size_t point_index, const double arg0,
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

  TestCase add_loops(const std::size_t path_index, const std::size_t point_index, const double arg0, const double arg1,
                     const std::size_t count) &&
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
  return {std::move(pv), {}, 0, fmt::format("{}-empty paths", path_count)};
}

TestCase ellipse(const std::size_t point_count, const bool closed, const bool no_tangents)
{
  const auto geometry = [point_count, no_tangents](const std::size_t i) {
    const double theta = M_PI * 2.0 * static_cast<double>(i) / static_cast<double>(point_count);
    const omm::Vec2f pos(std::cos(theta), std::sin(theta));
    if (no_tangents) {
      return omm::Point(pos);
    } else {
      const auto t_scale = 4.0 / 3.0 * std::tan(M_PI / (2.0 * static_cast<double>(point_count)));
      const auto t = omm::Vec2f(-std::sin(theta), std::cos(theta)) * static_cast<double>(t_scale);
      return omm::Point(pos, omm::PolarCoordinates(-t), omm::PolarCoordinates(t));
    }
  };

  const auto name = [point_count, closed, no_tangents]() {
    const auto s_open_closed = closed ? "closed" : "open";
    const auto s_interp = no_tangents ? "linear" : "smooth";
    return fmt::format("{}-Ellipse-{}-{}", point_count, s_open_closed, s_interp);
  };

  auto pv = std::make_unique<omm::PathVector>();
  auto& path = pv->add_path();
  std::deque<omm::DEdge> edges;
  path.set_single_point(std::make_shared<omm::PathPoint>(geometry(0), pv.get()));
  for (std::size_t i = 1; i < point_count; ++i) {
    auto new_point = std::make_shared<omm::PathPoint>(geometry(i), pv.get());
    edges.emplace_back(&path.add_edge(path.last_point(), std::move(new_point)), omm::Direction::Forward);
  }
  if (closed && point_count > 1) {
    edges.emplace_back(&path.add_edge(path.last_point(), path.first_point()), omm::Direction::Forward);
    return {std::move(pv), std::set{std::move(edges)}, 1, name()};
  }
  return {std::move(pv), {}, 1, name()};
}

TestCase rectangles(const std::size_t count)
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

TestCase leaf(std::vector<int> counts)
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

  const auto actual_faces = test_case.path_vector().faces();
  for (auto& face : test_case.expected_faces()) {
    std::cout << "E: " << face.to_string().toStdString() << std::endl;
  }
  for (auto& face : actual_faces) {
    std::cout << "A: " << face.to_string().toStdString() << std::endl;
  }
  ASSERT_EQ(test_case.expected_faces(), actual_faces);
}

TEST_P(GraphTest, ConnectedComponents)
{
  const auto& test_case = GetParam();
  EXPECT_EQ(omm::Graph(test_case.path_vector()).connected_components().size(), test_case.n_expected_components());
}

std::vector<omm::Point> linear_arm_geometry(const std::size_t length, const omm::Vec2f& direction)
{
  std::vector<omm::Point> ps;
  ps.reserve(length);
  for (std::size_t i = 0; i < length; ++i) {
    ps.emplace_back(omm::Point(static_cast<double>(i + 1) * direction));
  }
  return ps;
}

// clang-format off
#define EXPAND_ELLIPSE(N, ext) \
  ellipse(N, true, true) ext, \
  ellipse(N, false, true) ext, \
  ellipse(N, true, false) ext, \
  ellipse(N, false, false) ext

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
    leaf({2, 1, 1, 2})
);
// clang-format on

INSTANTIATE_TEST_SUITE_P(P, GraphTest, test_cases);
