#include "gtest/gtest.h"
#include "main/application.h"
#include "objects/pathobject.h"
#include "path/edge.h"
#include "path/face.h"
#include "path/graph.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "scene/disjointpathpointsetforest.h"
#include "scene/scene.h"
#include "testutil.h"

#include <QPainter>
#include <QSvgGenerator>


namespace
{

class EdgeLoop
{
public:
  explicit EdgeLoop(const std::size_t n)
    : m_path(m_path_vector.add_path(std::make_unique<omm::Path>()))
    , m_edges(create_edge_loop(n))
  {
  }

  const auto& edges() const { return m_edges; }

private:
  omm::PathVector m_path_vector;
  omm::Path& m_path;
  const std::deque<omm::Edge> m_edges;

  std::deque<omm::Edge> create_edge_loop(const std::size_t n) const
  {
    assert(n >= 2);
    std::deque<omm::Edge> edges(n);
    for (std::size_t i = 0; i < n; ++i) {
      edges.at(i).a = &m_path.add_point({});
      edges.at((i + 1) % n).b = edges.at(i).a;
    }
    return edges;
  }
};

omm::Face create_face(const std::deque<omm::Edge>& edges, const int offset, const bool reverse)
{
  std::deque<omm::Edge> es;
  std::rotate_copy(edges.begin(), edges.begin() + offset, edges.end(), std::back_insert_iterator(es));
  if (reverse) {
    std::reverse(es.begin(), es.end());
  }
  omm::Face face;
  for (const auto& edge : es) {
    static constexpr auto r = [](const omm::Edge& e) {
      omm::Edge r;
      r.a = e.b;
      r.b = e.a;
      return r;
    };
    face.add_edge(reverse ? r(edge) : edge);
  }
  return face;
}

double operator ""_u(long double d)
{
  return 80.0 * d;
}

double operator ""_deg(long double d)
{
  return d * M_PI / 180.0;
}

class FaceDetection : public ::testing::Test
{
protected:
  using Path = omm::Path;
  using Point = omm::Point;
  using Graph = omm::Graph;
  using Face = omm::Face;

  template<typename... Args> Path& add_path(Args&&... args)
  {
    return m_path_vector.add_path(std::make_unique<Path>(std::forward<Args>(args)...));
  }

  void join(const std::set<omm::PathPoint*, std::less<>>& joint)
  {
    m_path_vector.joined_points().insert(joint);
  }

  void expect_face(const std::vector<std::pair<int, int>>& indices)
  {
    omm::Face face;
    for (const auto& [ai, bi] : indices) {
      omm::Edge edge;
      edge.a = &m_path_vector.point_at_index(ai);
      edge.b = &m_path_vector.point_at_index(bi);
      face.add_edge(edge);
    }
    m_expected_faces.insert(face);
  }

  bool consistent_order(const Face& a, const Face& b)
  {
    // exactly one of them must be true.
    return (a == b) + (a < b) + (b < a) == 1;
  }

  template<typename Faces> bool consistent_order(const Faces& faces)
  {
    for (auto i = faces.begin(); i != faces.end(); std::advance(i, 1)) {
      for (auto j = std::next(i); j != faces.end(); std::advance(j, 1)) {
        if (!consistent_order(*i, *j)) {
          return false;
        }
      }
    }
    return true;
  }

  void to_svg()
  {
    QSvgGenerator canvas;
    canvas.setFileName("/tmp/pic.svg");
    QPainter painter{&canvas};

    for (const auto* path : m_path_vector.paths()) {
      painter.drawPath(path->to_painter_path());
    }
    painter.setPen(QColor{128, 0, 0});
    m_path_vector.draw_point_ids(painter);
  }

  void check()
  {
    // check if the operator< is consistent
    ASSERT_TRUE(consistent_order(m_expected_faces));

    const omm::Graph graph{m_path_vector};
    const auto actual_faces = graph.compute_faces();
    ASSERT_TRUE(consistent_order(actual_faces));
    LINFO << "detected faces:";
    for (const auto& f : actual_faces) {
      LINFO << f.to_string();
    }

    EXPECT_EQ(m_expected_faces, actual_faces);

    for (auto i = actual_faces.begin(); i != actual_faces.end(); std::advance(i, 1)) {
      for (auto j = std::next(i); j != actual_faces.end(); std::advance(j, 1)) {
        EXPECT_FALSE(i->contains(*j));
        EXPECT_FALSE(j->contains(*i));
      }
    }

    to_svg();
  }

private:
  ommtest::Application m_application;  // required to use QPainters text render engine
  omm::PathVector m_path_vector;
  std::set<Face> m_expected_faces;
};

}  // namespace

TEST(Path, FaceAddEdge)
{
  const EdgeLoop loop(4);

  static constexpr auto expect_true_perms = {
    std::array{0, 1, 2, 3},
    std::array{1, 2, 3, 0},
    std::array{3, 2, 1, 0},
    std::array{2, 1, 0, 3},
  };

  for (const auto permutation : expect_true_perms) {
    omm::Face face;
    for (const std::size_t i : permutation) {
      EXPECT_TRUE(face.add_edge(loop.edges().at(i)));
    }
  }

  // It adding the third edge is expected to fail because there's no way to orient it such that it
  // has a common point with the previous edge.
  // Hence, adding a fourth edge is not required.
  static constexpr auto expect_false_perms = {
    std::array{0, 1, 3},
    std::array{2, 1, 3},
    std::array{1, 2, 0},
    std::array{1, 0, 2},
  };

  for (const auto permutation : expect_false_perms) {
    omm::Face face;
    for (std::size_t k = 0; k < permutation.size() - 1; ++k) {
      EXPECT_TRUE(face.add_edge(loop.edges().at(permutation.at(k))));
    }
    EXPECT_FALSE(face.add_edge(loop.edges().at(permutation.back())));
  }
}

TEST(Path, FaceEquality)
{
  EdgeLoop loop(4);
  for (std::size_t i = 0; i < loop.edges().size(); ++i) {
    EXPECT_EQ(create_face(loop.edges(), 0, false), create_face(loop.edges(), i, false));
    EXPECT_EQ(create_face(loop.edges(), 0, true), create_face(loop.edges(), i, false));
    EXPECT_EQ(create_face(loop.edges(), i, true), create_face(loop.edges(), 0, true));
  }

  auto scrambled_edges = loop.edges();
  std::swap(scrambled_edges.at(0), scrambled_edges.at(1));
  for (std::size_t i = 0; i < loop.edges().size(); ++i) {
    EXPECT_NE(create_face(scrambled_edges, 0, false), create_face(loop.edges(), i, false));
    EXPECT_NE(create_face(scrambled_edges, 0, true), create_face(loop.edges(), i, false));
    EXPECT_NE(create_face(scrambled_edges, i, true), create_face(loop.edges(), 0, true));
  }
}

TEST_F(FaceDetection, A)
{
  //   (3)  --- (2,8) --- (7)
  //    |         |        |
  //    |         |        |
  //  (0,4) --- (1,5) --- (6)

  const auto as = add_path(std::deque{
                               Point{{0.0_u, 0.0_u}},  // 0
                               Point{{1.0_u, 0.0_u}},  // 1
                               Point{{1.0_u, 1.0_u}},  // 2
                               Point{{0.0_u, 1.0_u}},  // 3
                               Point{{0.0_u, 0.0_u}},  // 4
                           }).points();

  const auto bs = add_path(std::deque{
                               Point{{1.0_u, 0.0_u}},  // 5
                               Point{{2.0_u, 0.0_u}},  // 6
                               Point{{2.0_u, 1.0_u}},  // 7
                               Point{{1.0_u, 1.0_u}},  // 8
                           }).points();

  join({as[0], as[4]});
  join({as[1], bs[0]});
  join({as[2], bs[3]});
  expect_face({{0, 1}, {1, 2}, {2, 3}, {3, 4}});
  expect_face({{5, 6}, {6, 7}, {7, 8}, {1, 2}});
  check();
}

TEST_F(FaceDetection, B)
{
  //    +-- (1,5) --+
  //    |     |     |
  //    |     |    (4)
  //    |     |     |
  //    +- (0,2,3) -+

  using PC = omm::PolarCoordinates;
  const auto& as = add_path(std::deque{
                                Point{{0.0_u, 0.0_u}, PC{}, PC{180.0_deg, 1.0_u}}, // 0
                                Point{{0.0_u, 2.0_u}, PC{180.0_deg, 1.0_u}, PC{-90.0_deg, 1.0_u}},  // 1
                                Point{{0.0_u, 0.0_u}, PC{90.0_deg, 1.0_u}, PC{}},  // 2
                            }).points();
  const auto& bs = add_path(std::deque{
                                Point{{0.0_u, 0.0_u}, PC{}, PC{0.0_deg, 1.0_u}},  // 3
                                Point{{1.0_u, 1.0_u}, PC{-90.0_deg, 1.0_u}, PC{90.0_deg, 1.0_u}},  // 4
                                Point{{0.0_u, 2.0_u}, PC{0.0_deg, 1.0_u}, PC{}},  // 5
                            }).points();

  join({as[0], as[2], bs[0]});
  join({as[1], bs[2]});
  expect_face({{0, 1}, {1, 2}});
  expect_face({{3, 4}, {4, 5}, {}});
  check();
}
