#include "gtest/gtest.h"
#include "path/pathvector.h"
#include "path/path.h"
#include "path/graph.h"
#include "path/edge.h"
#include "path/face.h"
#include "scene/disjointpathpointsetforest.h"

using namespace omm;

auto make_face(const PathVector& pv, const std::vector<std::pair<int, int>>& indices)
{
  Face face;
  for (const auto& [ai, bi] : indices) {
    Edge edge;
    edge.a = &pv.point_at_index(ai);
    edge.b = &pv.point_at_index(bi);
    face.add_edge(edge);
  }
  return face;
}

// Considering the loop without joints (A) --e1-- (B) --e2-- (A), e1 and e2 will be considered equal.
// However, this case does not occur during testing.
bool edge_equal(const Edge& a, const Edge& b)
{
  return std::set{a.a, a.b} == std::set{b.a, b.b};
}

bool operator==(const Face& a, const Face& b)
{
  const auto& a_edges = a.edges();
  const auto& b_edges = b.edges();
  if (a_edges.size() != b_edges.size()) {
    return false;
  }

  const auto n = a_edges.size();
  const auto rotation_match = [n, &a_edges, &b_edges](const int offset, bool reverse) {
    for (std::size_t j = 0; j < n; ++j) {
      const auto ai = reverse ? n - j - 1 : j;
      const auto bi = (j + offset) % n;
      if (!edge_equal(a_edges[ai], b_edges[bi])) {
        return false;
      }
    }
    return true;
  };

  for (std::size_t i = 0; i < n; ++i) {
    if (rotation_match(i, false) || rotation_match(i, true)) {
      return true;
    }
  }

  return false;
}

TEST(Path, face_detection)
{

  PathVector path_vector;

  // define following path vector:
  //
  //   (3)  --- (2,8) --- (7)
  //    |         |        |
  //    |         |        |
  //  (0,4) --- (1,5) --- (6)

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
  const auto dot = graph.to_dot();
  LINFO << dot;

  const auto faces = graph.compute_faces();
  ASSERT_EQ(faces.size(), 2);
  ASSERT_TRUE(faces[0] == make_face(path_vector, {{0, 1}, {1, 2}, {2, 3}, {3, 4}}));
  ASSERT_TRUE(faces[1] == make_face(path_vector, {{5, 6}, {6, 7}, {7, 8}, {1, 2}}));
}
