#include "gtest/gtest.h"
#include "logging.h"
#include "common.h"
#include <QDebug>
#include <map>

namespace
{

struct Vertex
{
  Vertex(int id) : id(id) {}
  std::set<Vertex*> successors;
  operator int() const { return id; }
  bool operator==(const Vertex& other) { return other.id == id; }
private:
  int id;
};

std::map<int, Vertex*> test_case()
{
  // Wikipedia has a drawing of our test graph:
  // https://en.wikipedia.org/wiki/Topological_sorting#/media/File:Directed_acyclic_graph_2.svg

  std::map<int, Vertex*> vertices;
  for (int id : { 2, 3, 5, 7, 8, 9, 10, 11 }) {
    vertices.insert({ id, new Vertex(id) });
  }

  vertices[5]->successors.insert(vertices[11]);
  vertices[11]->successors.insert(vertices[2]);
  vertices[11]->successors.insert(vertices[9]);
  vertices[11]->successors.insert(vertices[10]);
  vertices[7]->successors.insert(vertices[11]);
  vertices[7]->successors.insert(vertices[8]);
  vertices[3]->successors.insert(vertices[8]);
  vertices[3]->successors.insert(vertices[10]);
  vertices[8]->successors.insert(vertices[9]);

  assert(vertices.size() == 8);
  return vertices;
}

void clean_up_test_case(std::map<int, Vertex*> vertices)
{
  for (auto [_ ,v] : vertices) {
    delete v;
  }
}

std::set<Vertex*> get_successors(const Vertex* v)
{
  return v->successors;
}

}  // namespace

TEST(common, find_path)
{
  const auto vs = test_case();

#define check_path(start_id, end_id, expect_exists) \
  do { \
    std::list<Vertex*> path; \
    Vertex* start = vs.at(start_id); \
    Vertex* end = vs.at(end_id); \
    bool actual_exists = omm::find_path<Vertex*>(start, end, path, get_successors); \
    EXPECT_EQ(actual_exists, expect_exists); \
  } while (false)

  check_path(5,  7,  false);
  check_path(5,  3,  false);
  check_path(5,  11, true);
  check_path(5,  8,  false);
  check_path(5,  2,  true);
  check_path(5,  9,  true);
  check_path(5,  10, true);

  check_path(7,  5,  false);
  check_path(7,  3,  false);
  check_path(7,  11, true);
  check_path(7,  8,  true);
  check_path(7,  2,  true);
  check_path(7,  9,  true);
  check_path(7,  10, true);

  check_path(3,  7,  false);
  check_path(3,  5,  false);
  check_path(3,  11, false);
  check_path(3,  8,  true);
  check_path(3,  2,  false);
  check_path(3,  9,  true);
  check_path(3,  10, true);

  check_path(11, 7,  false);
  check_path(11, 3,  false);
  check_path(11, 5,  false);
  check_path(11, 8,  false);
  check_path(11, 2,  true);
  check_path(11, 9,  true);
  check_path(11, 10, true);

  check_path(8,  7,  false);
  check_path(8,  3,  false);
  check_path(8,  11, false);
  check_path(8,  5,  false);
  check_path(8,  2,  false);
  check_path(8,  9,  true);
  check_path(8,  10, false);

  check_path(2,  7,  false);
  check_path(2,  3,  false);
  check_path(2,  11, false);
  check_path(2,  8,  false);
  check_path(2,  5,  false);
  check_path(2,  9,  false);
  check_path(2,  10, false);

  check_path(9,  7,  false);
  check_path(9,  3,  false);
  check_path(9,  11, false);
  check_path(9,  8,  false);
  check_path(9,  2,  false);
  check_path(9,  5,  false);
  check_path(9,  10, false);

  check_path(10, 7,  false);
  check_path(10, 3,  false);
  check_path(10, 11, false);
  check_path(10, 8,  false);
  check_path(10, 2,  false);
  check_path(10, 9,  false);
  check_path(10, 5,  false);

  clean_up_test_case(vs);
}

TEST(common, tsort)
{
  const auto vs = test_case();

  std::set<Vertex*> vertices;
  for (auto [id, v] : vs) {
    vertices.insert(v);
  }
  const auto [ has_cycle, sequence ] = omm::topological_sort<Vertex*>(vertices, get_successors);
  const std::vector v_seq(sequence.begin(), sequence.end());
  for (std::size_t i = 0; i < v_seq.size(); ++i) {
    for (std::size_t j = 0; j < i; ++j) {
      std::list<Vertex*> path_i_j;
      // there must not be edges from back to front.
      EXPECT_FALSE(omm::find_path<Vertex*>(v_seq.at(i), v_seq.at(j), path_i_j, get_successors));
    }
  }

  clean_up_test_case(vs);
}
