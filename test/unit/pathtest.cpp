#include "objects/path.h"
#include "objects/segment.h"
#include "gtest/gtest.h"
#include <2geom/2geom.h>

using namespace omm;

#define expect_segment_eq(a, b) \
  EXPECT_EQ((a).size(), (b).size()); \
  for (std::size_t i = 0; i < (a).size(); ++i) { \
    if (i > 0) { \
      EXPECT_EQ((a)[i].left_position(), (b)[i].left_position()); \
    } \
    if (i < (a).size() - 1) { \
      EXPECT_EQ((a)[i].right_position(), (b)[i].right_position()); \
    } \
    EXPECT_EQ((a)[i].position, (b)[i].position); \
  }

TEST(Path, set)
{
  Path path(nullptr);
  path.property(Path::INTERPOLATION_PROPERTY_KEY)->set(InterpolationMode::Bezier);

  const Segment segment0{Point({0, 0}), Point({1, 1}), Point({2, 2})};
  const Segment segment1{Point({2, 0}), Point({1, 1}), Point({2, 0})};
  path.segments.push_back(segment0);
  path.segments.push_back(segment1);

  expect_segment_eq(path.segments[0], segment0);
  expect_segment_eq(path.segments[1], segment1);
  EXPECT_EQ(std::distance(path.begin(), path.end()), 6);

  path.set(path.geom_paths());
  expect_segment_eq(path.segments[0], segment0);
  expect_segment_eq(path.segments[1], segment1);
  EXPECT_EQ(std::distance(path.begin(), path.end()), 6);
}
