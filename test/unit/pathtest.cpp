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
}
