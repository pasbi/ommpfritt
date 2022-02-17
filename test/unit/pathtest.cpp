#include "gtest/gtest.h"
#include "path/pathvector.h"
#include "path/path.h"
#include "path/graph.h"
#include "path/edge.h"
#include "path/face.h"
#include "path/pathpoint.h"
#include "scene/disjointpathpointsetforest.h"

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

auto make_face(const omm::PathVector& pv, const std::vector<std::pair<int, int>>& indices)
{
  omm::Face face;
  for (const auto& [ai, bi] : indices) {
    omm::Edge edge;
    edge.a = &pv.point_at_index(ai);
    edge.b = &pv.point_at_index(bi);
    face.add_edge(edge);
  }
  return face;
}

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

TEST(Path, face_detection)
{

  omm::PathVector path_vector;

  // define following path vector:
  //
  //   (3)  --- (2,8) --- (7)
  //    |         |        |
  //    |         |        |
  //  (0,4) --- (1,5) --- (6)

  using omm::Path;
  using omm::Point;
  using omm::Graph;

  const auto as = path_vector.add_path(std::make_unique<Path>(std::deque<Point>{
                                           Point{{0.0, 0.0}},  // 0
                                           Point{{1.0, 0.0}},  // 1
                                           Point{{1.0, 1.0}},  // 2
                                           Point{{0.0, 1.0}},  // 3
                                           Point{{0.0, 0.0}},  // 4
                                       })).points();

  const auto bs = path_vector.add_path(std::make_unique<Path>(std::deque<Point>{
                                           Point{{1.0, 0.0}},  // 5
                                           Point{{2.0, 0.0}},  // 6
                                           Point{{2.0, 1.0}},  // 7
                                           Point{{1.0, 1.0}},  // 8
                                       })).points();

  path_vector.joined_points().insert({as[0], as[4]});
  path_vector.joined_points().insert({as[1], bs[0]});
  path_vector.joined_points().insert({as[2], bs[3]});

  const Graph graph{path_vector};
  const auto faces = graph.compute_faces();
  ASSERT_EQ(faces.size(), 2);
  ASSERT_EQ(faces[0], make_face(path_vector, {{0, 1}, {1, 2}, {2, 3}, {3, 4}}));
  ASSERT_EQ(faces[1], make_face(path_vector, {{5, 6}, {6, 7}, {7, 8}, {1, 2}}));
}
