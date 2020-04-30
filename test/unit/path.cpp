#include "gtest/gtest.h"
#include <string>
#include "objects/path.h"
#include "common.h"
#include "logging.h"

bool operator==(const std::vector<omm::Point*>& lhs, const std::vector<omm::Point>& rhs)
{
  if (lhs.size() != rhs.size()) {
    return false;
  } else {
    for (std::size_t i = 0; i < lhs.size(); ++i) { if (*lhs[i] != rhs[i]) { return false; }  }
  }
  return true;
}

void prepare_indices(std::vector<std::size_t>& indices)
{
  for (std::size_t i = 0; i < indices.size(); ++i) {
    indices[i] -= i;
  }
}

/**
 * @brief remove_add_points remove @code indices from @code initial_points then add them again.
 *  Test some invariants.
 * @param initial_points the base set of points
 * @param indices the points to remove from the path
 */
void remove_add_points( const std::vector<omm::Point>& initial_points,
                        std::vector<std::size_t> indices)
{
  prepare_indices(indices);
  omm::Path path(nullptr);
  path.set_points(initial_points);
  const auto sequences = path.remove_points(indices);
  ASSERT_EQ( path.points().size(), initial_points.size() - indices.size() );
  path.add_points(sequences);

  ASSERT_EQ( path.points().size(), initial_points.size() );
  for (std::size_t i = 0; i < initial_points.size(); ++i) {
    ASSERT_EQ( path.points()[i], initial_points[i] );
  }
}

void test_invariant_1( const std::vector<omm::Point>& initial_points,
                       std::vector<std::size_t> indices)
{
  prepare_indices(indices);
  omm::Scene* scene = nullptr;
  omm::Path path(scene);
  path.set_points(initial_points);

  const auto sequences = path.remove_points(indices);
  path.add_points(sequences);
  EXPECT_TRUE(path.points() == initial_points);
}

void test_invariant_2( const std::vector<omm::Point>& initial_points,
                       const std::vector<omm::Path::PointSequence>& sequences )
{
  omm::Scene* scene = nullptr;
  omm::Path path(scene);
  path.set_points(initial_points);

  std::vector<std::size_t> indices = path.add_points(sequences);
  path.remove_points(indices);

  EXPECT_TRUE(path.points() == initial_points);
}

TEST(path, remove_add_points)
{
  static const std::vector<omm::Point> points3 ({
    omm::Point({0, 0}),
    omm::Point({1, 0}),
    omm::Point({2, 0})
  });

  static const std::vector<omm::Point> points10 ({
    omm::Point({0, 0}),
    omm::Point({1, 0}),
    omm::Point({2, 0}),
    omm::Point({3, 0}),
    omm::Point({4, 0}),
    omm::Point({5, 0}),
    omm::Point({6, 0}),
    omm::Point({7, 0}),
    omm::Point({8, 0}),
    omm::Point({9, 0}),
  });

  ASSERT_DEATH(remove_add_points(points3, { 4, 4 }), "");

  remove_add_points(points3, {});
  remove_add_points(points3, { 0, 1, 2 });
  remove_add_points(points10, { 0, 1 });
  remove_add_points(points10, { 0, 2 });
  remove_add_points(points10, { 0, 1, 3, 5 });
  remove_add_points(points10, { 0, 1, 2, 3 });
  remove_add_points(points10, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
  remove_add_points(points10, { 0, 1, 2, 3, 4, 6, 7, 8, 9 });
  remove_add_points(points10, { 0, 1, 2, 3, 6, 7, 8, 9 });
  remove_add_points(points10, { 0, 2, 4, 6, 8 });
  remove_add_points(points10, { 1, 3, 5, 7, 9 });
  remove_add_points(points10, { 2, 4 });
  remove_add_points(points10, { 2, 3 });
  remove_add_points(points10, { 2, 9 });
  remove_add_points(points10, { 2, 9 });
  remove_add_points(points10, { 7, 9 });
  remove_add_points(points10, { 8, 9 });

  test_invariant_1(points3, {});
  test_invariant_1(points3, {0, 1, 2});
  test_invariant_1(points3, {1});
  test_invariant_1(points10, {0, 4, 6, 8});
  test_invariant_1(points10, {0, 1, 2, 3, 4, 6, 8});
  test_invariant_1(points10, {5, 6, 7, 9});
  test_invariant_1(points10, {5, 6, 7, 8, 9});
  test_invariant_1(points10, {0, 6, 7, 8, 9});

  test_invariant_2(points3, {
    omm::Path::PointSequence{ 0, { omm::Point(omm::Vec2f(0, 1)) } }
  });

  test_invariant_2(points3, {
    omm::Path::PointSequence{ 1, { omm::Point(omm::Vec2f(0, 1)) } }
  });

  test_invariant_2(points3, {
    omm::Path::PointSequence{ 2, { omm::Point(omm::Vec2f(0, 1)) } }
  });

  test_invariant_2(points3, {
    omm::Path::PointSequence{ 0, { omm::Point(omm::Vec2f(0, 1)), omm::Point(omm::Vec2f(0, 2)) } }
  });

  test_invariant_2(points3, {
    omm::Path::PointSequence{ 0, { omm::Point(omm::Vec2f(0, 1)),
                                   omm::Point(omm::Vec2f(0, 2)) } },
    omm::Path::PointSequence{ 1, { omm::Point(omm::Vec2f(0, 3)),
                                   omm::Point(omm::Vec2f(0, 4)) } } });

  test_invariant_2(points3, {
    omm::Path::PointSequence{ 0, { omm::Point(omm::Vec2f(0, 1)),
                                   omm::Point(omm::Vec2f(0, 2)) } },
    omm::Path::PointSequence{ 2, { omm::Point(omm::Vec2f(0, 3)),
                                   omm::Point(omm::Vec2f(0, 4)) } } });

  test_invariant_2(points3, {
    omm::Path::PointSequence{ 0, { omm::Point(omm::Vec2f(0, 1)),
                                   omm::Point(omm::Vec2f(0, 2)) } },
    omm::Path::PointSequence{ 3, { omm::Point(omm::Vec2f(0, 3)),
                                   omm::Point(omm::Vec2f(0, 4)) } } });

  test_invariant_2(points3, {
    omm::Path::PointSequence{ 0, { omm::Point(omm::Vec2f(0, 1)),
                                   omm::Point(omm::Vec2f(0, 2)) } },
    omm::Path::PointSequence{ 3, { omm::Point(omm::Vec2f(0, 3)),
                                   omm::Point(omm::Vec2f(0, 3)),
                                   omm::Point(omm::Vec2f(0, 5)) } } });
}
