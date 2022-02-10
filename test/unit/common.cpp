#include "common.h"
#include "logging.h"
#include "disjointset.h"
#include "gtest/gtest.h"
#include <QDebug>
#include <map>

namespace
{
struct Vertex {
  explicit Vertex(int id) : id(id)
  {
  }
  std::set<Vertex*> successors;
  operator int() const
  {
    return id;
  }
  bool operator==(const Vertex& other) const
  {
    return other.id == id;
  }

private:
  int id;
};

using id_type = int;
using test_case_type = std::map<id_type, std::unique_ptr<Vertex>>;

test_case_type make_test(const std::set<int>& ids)
{
  test_case_type vertices;
  for (int id : ids) {
    vertices.insert({id, std::make_unique<Vertex>(id)});
  }
  return vertices;
}

void connect(const test_case_type& vertices, id_type from, id_type to)
{
  vertices.at(from)->successors.insert(vertices.at(to).get());
}

test_case_type wiki_test_case()
{
  // Wikipedia has a drawing of test test graph:
  // https://en.wikipedia.org/wiki/Topological_sorting#/media/File:Directed_acyclic_graph_2.svg

  test_case_type vertices = make_test({2, 3, 5, 7, 8, 9, 10, 11});
  connect(vertices, 5, 11);
  connect(vertices, 11, 2);
  connect(vertices, 11, 9);
  connect(vertices, 11, 10);
  connect(vertices, 7, 11);
  connect(vertices, 7, 8);
  connect(vertices, 3, 8);
  connect(vertices, 3, 10);
  connect(vertices, 8, 9);

  assert(vertices.size() == 8);
  return vertices;
}

test_case_type empty_test_case()
{
  return make_test({});
}

test_case_type single_vertex_test_case()
{
  return make_test({0});
}

test_case_type cycle_test_case()
{
  test_case_type vertices = make_test({0, 1});
  connect(vertices, 0, 1);
  connect(vertices, 1, 0);
  return vertices;
}

template<typename T> std::set<Vertex*> get_vertices(const std::map<T, std::unique_ptr<Vertex>>& map)
{
  return util::transform<std::set>(map, [](auto&& p) { return p.second.get(); });
}

std::set<Vertex*> get_successors(const Vertex* v)
{
  return v->successors;
}

decltype(auto) topological_sort(const test_case_type& test_case)
{
  return omm::topological_sort<Vertex*>(get_vertices(test_case), get_successors);
}

}  // namespace

TEST(common, find_path)
{
  const auto test_case = wiki_test_case();

#define check_path(start_id, end_id, expect_exists) \
  do { \
    std::list<Vertex*> path; \
    Vertex& start = *test_case.at(start_id); \
    Vertex& end = *test_case.at(end_id); \
    bool actual_exists = omm::find_path<Vertex*>(&start, &end, path, get_successors); \
    EXPECT_EQ(actual_exists, expect_exists); \
  } while (false)

  check_path(5, 7, false);
  check_path(5, 3, false);
  check_path(5, 11, true);
  check_path(5, 8, false);
  check_path(5, 2, true);
  check_path(5, 9, true);
  check_path(5, 10, true);

  check_path(7, 5, false);
  check_path(7, 3, false);
  check_path(7, 11, true);
  check_path(7, 8, true);
  check_path(7, 2, true);
  check_path(7, 9, true);
  check_path(7, 10, true);

  check_path(3, 7, false);
  check_path(3, 5, false);
  check_path(3, 11, false);
  check_path(3, 8, true);
  check_path(3, 2, false);
  check_path(3, 9, true);
  check_path(3, 10, true);

  check_path(11, 7, false);
  check_path(11, 3, false);
  check_path(11, 5, false);
  check_path(11, 8, false);
  check_path(11, 2, true);
  check_path(11, 9, true);
  check_path(11, 10, true);

  check_path(8, 7, false);
  check_path(8, 3, false);
  check_path(8, 11, false);
  check_path(8, 5, false);
  check_path(8, 2, false);
  check_path(8, 9, true);
  check_path(8, 10, false);

  check_path(2, 7, false);
  check_path(2, 3, false);
  check_path(2, 11, false);
  check_path(2, 8, false);
  check_path(2, 5, false);
  check_path(2, 9, false);
  check_path(2, 10, false);

  check_path(9, 7, false);
  check_path(9, 3, false);
  check_path(9, 11, false);
  check_path(9, 8, false);
  check_path(9, 2, false);
  check_path(9, 5, false);
  check_path(9, 10, false);

  check_path(10, 7, false);
  check_path(10, 3, false);
  check_path(10, 11, false);
  check_path(10, 8, false);
  check_path(10, 2, false);
  check_path(10, 9, false);
  check_path(10, 5, false);
}

TEST(common, tsort_simple)
{
  {
    const auto [has_cycle, sequence] = ::topological_sort(empty_test_case());
    EXPECT_FALSE(has_cycle);
    EXPECT_TRUE(sequence.empty());
  }
  {
    const auto [has_cycle, sequence] = ::topological_sort(single_vertex_test_case());
    EXPECT_FALSE(has_cycle);
  }
}

TEST(common, tsort_cycle)
{
  {
    const auto [has_cycle, sequence] = topological_sort(cycle_test_case());
    EXPECT_TRUE(has_cycle);
  }
}

TEST(common, tsort)
{
  const auto test_case = wiki_test_case();
  const auto [has_cycle, sequence] = topological_sort(test_case);
  const std::vector v_seq(sequence.begin(), sequence.end());
  for (std::size_t i = 0; i < v_seq.size(); ++i) {
    for (std::size_t j = 0; j < i; ++j) {
      std::list<Vertex*> path_i_j;
      // there must not be edges from back to front.
      EXPECT_FALSE(omm::find_path<Vertex*>(v_seq.at(i), v_seq.at(j), path_i_j, get_successors));
    }
  }
  EXPECT_FALSE(has_cycle);
}

TEST(common, modern_cpp_for_loop_qt_cow_container)
{
  [[maybe_unused]] static bool begin_was_called = false;
  [[maybe_unused]] static bool end_was_called = false;
  [[maybe_unused]] static bool const_begin_was_called = false;
  [[maybe_unused]] static bool const_end_was_called = false;
  const auto reset = []() {
    begin_was_called = true;
    end_was_called = true;
    const_begin_was_called = true;
    const_end_was_called = true;
  };

  class TestContainer
  {
  private:
    std::vector<int> dummy;

  public:
    decltype(auto) begin()
    {
      begin_was_called = true;
      return dummy.begin();
    };

    decltype(auto) end()
    {
      end_was_called = true;
      return dummy.end();
    };

    [[nodiscard]] decltype(auto) begin() const
    {
      const_begin_was_called = true;
      return dummy.cbegin();
    };

    [[nodiscard]] decltype(auto) end() const
    {
      const_end_was_called = true;
      return dummy.cend();
    };
  };

  auto f = []() { return TestContainer(); };

  reset();
  for (auto&& i : f()) {
    Q_UNUSED(i)
    EXPECT_TRUE(begin_was_called);
    EXPECT_TRUE(end_was_called);
    EXPECT_FALSE(const_begin_was_called);
    EXPECT_FALSE(const_end_was_called);
  }

  reset();
  const auto fs = f();
  for (auto&& i : fs) {
    Q_UNUSED(i)
    EXPECT_FALSE(begin_was_called);
    EXPECT_FALSE(end_was_called);
    EXPECT_TRUE(const_begin_was_called);
    EXPECT_TRUE(const_end_was_called);
  }
}

TEST(common, coherent_ranges)
{
  const std::vector values{0, 1, 2, 3, 4, 5, 6, 7, 8};

  EXPECT_TRUE(omm::find_coherent_ranges(std::vector<int>{}, [](auto&&) { return true; }).empty());

  static constexpr auto is_even = [](int i) { return i % 2 == 0; };
  const auto even_ranges = omm::find_coherent_ranges(values, is_even);
  ASSERT_EQ(even_ranges.size(), 5);
  for (std::size_t i = 0; i < even_ranges.size(); ++i) {
    EXPECT_EQ(even_ranges[i].start, 2 * i);
    EXPECT_EQ(even_ranges[i].size, 1);
  }

  static constexpr auto is_small = [](int i) { return i < 5; };
  const auto small_ranges = omm::find_coherent_ranges(values, is_small);
  ASSERT_EQ(small_ranges.size(), 1);
  EXPECT_EQ(small_ranges[0].start, 0);
  EXPECT_EQ(small_ranges[0].size, 5);

  static constexpr auto is_big = [](int i) { return i > 5; };
  const auto big_ranges = omm::find_coherent_ranges(values, is_big);
  ASSERT_EQ(big_ranges.size(), 1);
  EXPECT_EQ(big_ranges[0].start, 6);
  EXPECT_EQ(big_ranges[0].size, 3);

  static constexpr auto is_prime_power = [](int i) { return std::set{2, 3, 4, 5, 7, 8}.contains(i); };
  const auto prime_power_ranges = omm::find_coherent_ranges(values, is_prime_power);
  ASSERT_EQ(prime_power_ranges.size(), 2);
  EXPECT_EQ(prime_power_ranges[0].start, 2);
  EXPECT_EQ(prime_power_ranges[0].size, 4);
  EXPECT_EQ(prime_power_ranges[1].start, 7);
  EXPECT_EQ(prime_power_ranges[1].size, 2);
}

TEST(common, disjoint_set_forest)
{
  EXPECT_TRUE(omm::DisjointSetForest<int>::sets_disjoint({1, 2, 3}, {4, 5, 6}));
  EXPECT_FALSE(omm::DisjointSetForest<int>::sets_disjoint({1, 2, 3}, {3, 5, 6}));
  EXPECT_TRUE(omm::DisjointSetForest<int>::sets_disjoint({}, {3, 5, 6}));
  EXPECT_TRUE(omm::DisjointSetForest<int>::sets_disjoint({3, 5, 6}, {}));

  omm::DisjointSetForest<int> cluster;
  EXPECT_EQ(cluster.get(42), ::transparent_set<int>{});
  cluster.insert({1, 2 ,3});
  EXPECT_EQ(cluster.get(1), (::transparent_set<int>{1, 2, 3}));
  EXPECT_EQ(cluster.get(2), (::transparent_set<int>{1, 2, 3}));
  EXPECT_EQ(cluster.get(0), ::transparent_set<int>{});

  cluster.insert({4, 5});
  EXPECT_EQ(cluster.get(1), (::transparent_set<int>{1, 2, 3}));
  EXPECT_EQ(cluster.get(0), ::transparent_set<int>{});
  EXPECT_EQ(cluster.get(4), (::transparent_set<int>{4, 5}));

  cluster.remove({4});
  EXPECT_EQ(cluster.get(1), (::transparent_set<int>{1, 2, 3}));
  EXPECT_EQ(cluster.get(4), ::transparent_set<int>{});
  EXPECT_EQ(cluster.get(5), ::transparent_set<int>{});

  cluster.insert({4, 5});
  cluster.insert({1, 6, 7});
  EXPECT_EQ(cluster.get(1), (::transparent_set<int>{1, 2, 3, 6, 7}));
  EXPECT_EQ(cluster.get(6), (::transparent_set<int>{1, 2, 3, 6, 7}));

  cluster.insert({1, 4});
  EXPECT_EQ(cluster.get(1), (::transparent_set<int>{1, 2, 3, 4, 5, 6, 7}));
  EXPECT_EQ(cluster.get(3), (::transparent_set<int>{1, 2, 3, 4, 5, 6, 7}));

  cluster.remove({4});
  EXPECT_EQ(cluster.get(1), ::transparent_set<int>{});
  EXPECT_EQ(cluster.get(4), ::transparent_set<int>{});
  EXPECT_EQ(cluster.get(5), ::transparent_set<int>{});
}
