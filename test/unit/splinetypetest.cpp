#include "gtest/gtest.h"
#include "splinetype.h"

TEST(SplineTypeTest, insert)
{
  using namespace omm;
  using Knot = SplineType::Knot;
  omm::SplineType st;

  {
    const Knot knot(1.0, 2.0, 3.0);
    st.knots.insert({0.5, knot});
    ASSERT_EQ(st.knots.begin()->second, knot);
  }

  {
    const Knot knot(1.1, 2.1, 3.1);
    st.knots.insert({0.9, knot});
    ASSERT_EQ(std::next(st.knots.begin())->second, knot);
  }

  {
    const Knot knot(1.2, 2.2, 3.2);
    st.knots.insert({0.1, knot});
    ASSERT_EQ(st.knots.begin()->second, knot);
  }

  {
    const Knot knot(1.3, 2.3, 3.3);
    st.knots.insert({0.1, knot});
    ASSERT_EQ(st.knots.begin()->second, knot);
  }

  {
    const Knot knot(1.4, 2.4, 3.4);
    st.knots.insert({0.9, knot});
    EXPECT_EQ(std::next(st.knots.begin(), 2)->second, knot);
  }

}

