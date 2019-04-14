#include <gtest/gtest.h>
#include <glog/logging.h>
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

void remove_add_points( const std::vector<omm::Point>& initial_points,
                        const std::vector<std::size_t>& indices)
{
  omm::Scene* scene = nullptr;
  omm::Path path(scene);
  path.set_points(initial_points);

  LINFO << "=============\n\n\n";
  LINFO << "original:     " << path.points();
  const auto sequences = path.remove_points(indices);
  LINFO << "after remove: " << path.points();
  ASSERT_EQ( path.points().size(), initial_points.size() - indices.size() );


  LINFO << "sequences:";
  for (const auto& sequence : sequences) {
    LINFO << " >> " << sequence.position << " " << sequence.sequence;
  }

  std::size_t i = 0;
  for (const auto& sequence : sequences) {
    const std::size_t pos = sequence.position;
    if (i == pos) {
      for (auto it = sequence.sequence.begin(); it != sequence.sequence.end(); ++it) {
        EXPECT_EQ(*it, initial_points[i]);
        i++;
      }
    } else {
      i++;
    }
  }

  path.add_points(sequences);

  LINFO << "after insert: " << path.points();

  ASSERT_EQ( path.points().size(), initial_points.size() );
  for (std::size_t i = 0; i < initial_points.size(); ++i) {
    ASSERT_EQ( path.points()[i], initial_points[i] );
  }
}

void test_invariant_1( const std::vector<omm::Point>& initial_points,
                       const std::vector<std::size_t>& indices)
{
  omm::Scene* scene = nullptr;
  omm::Path path(scene);
  path.set_points(initial_points);

  LINFO << "=============\n\n\n";
  LINFO << "original:     " << path.points();
  const auto sequences = path.remove_points(indices);
  LINFO << "after remove: " << path.points();
  LINFO << "sequences: " << sequences.size();
  for (const auto& sequence : sequences) {
    LINFO << " >> " << sequence.position << " " << sequence.sequence;
  }
  path.add_points(sequences);
  LINFO << "after insert: " << path.points();
  EXPECT_TRUE(path.points() == initial_points);
}

void test_invariant_2( const std::vector<omm::Point> initial_points,
                       const std::vector<omm::Path::PointSequence>& sequences )
{
  omm::Scene* scene = nullptr;
  omm::Path path(scene);
  path.set_points(initial_points);

  LINFO << "=============\n\n\n";
  LINFO << "original:     " << path.points();
  LINFO << "sequences: " << sequences.size();
  for (const auto& sequence : sequences) {
    LINFO << " >> " << sequence.position << " " << sequence.sequence;
  }
  std::vector<std::size_t> indices = path.add_points(sequences);
  LINFO << "indices: " << indices.size() << " " << indices;
  LINFO << "after insert: " << path.points();
  path.remove_points(indices);
  LINFO << "after remove: " << path.points();


  EXPECT_TRUE(path.points() == initial_points);
}

TEST(path, remove_add_points)
{
  static const std::vector<omm::Point> points3 ({
    omm::Point(omm::Vec2f(0, 0)),
    omm::Point(omm::Vec2f(1, 0)),
    omm::Point(omm::Vec2f(2, 0))
  });

  static const std::vector<omm::Point> points10 ({
    omm::Point(omm::Vec2f(0, 0)),
    omm::Point(omm::Vec2f(1, 0)),
    omm::Point(omm::Vec2f(2, 0)),
    omm::Point(omm::Vec2f(3, 0)),
    omm::Point(omm::Vec2f(4, 0)),
    omm::Point(omm::Vec2f(5, 0)),
    omm::Point(omm::Vec2f(6, 0)),
    omm::Point(omm::Vec2f(7, 0)),
    omm::Point(omm::Vec2f(8, 0)),
    omm::Point(omm::Vec2f(9, 0)),
  });

  // assertion shall fail if candidate points are in wrong order
  ASSERT_DEATH(remove_add_points(points3, { 1, 0 }), "");
  ASSERT_DEATH(remove_add_points(points3, { 2, 0 }), "");
  ASSERT_DEATH(remove_add_points(points10, { 0, 1, 3, 2 }), "");
  ASSERT_DEATH(remove_add_points(points10, { 1, 0, 2, 3 }), "");
  ASSERT_DEATH(remove_add_points(points3, { 4, 4 }), "");

  // remove shall succeed if order is correct
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

  ASSERT_DEATH( // interleaving sequences
    test_invariant_2(points3, {
      omm::Path::PointSequence{ 0, { omm::Point(omm::Vec2f(0, 1)),
                                     omm::Point(omm::Vec2f(0, 2)) } },
      omm::Path::PointSequence{ 1, { omm::Point(omm::Vec2f(0, 3)),
                                     omm::Point(omm::Vec2f(0, 4)) } }
    }) , "");

  ASSERT_DEATH( // subsequent sequences
    test_invariant_2(points3, {
      omm::Path::PointSequence{ 0, { omm::Point(omm::Vec2f(0, 1)),
                                     omm::Point(omm::Vec2f(0, 2)) } },
      omm::Path::PointSequence{ 2, { omm::Point(omm::Vec2f(0, 3)),
                                     omm::Point(omm::Vec2f(0, 4)) } }
    }) , "");

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
