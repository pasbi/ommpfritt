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
    ASSERT_EQ(std::next(st.knots.begin())->second, knot);
  }

  {
    const Knot knot(1.4, 2.4, 3.4);
    st.knots.insert({0.9, knot});
    EXPECT_EQ(std::next(st.knots.begin(), 4)->second, knot);
  }
}

TEST(SplineTypeTest, value)
{
  const omm::SplineType up(omm::SplineType::Initialization::Linear, false);
  const omm::SplineType down(omm::SplineType::Initialization::Linear, true);
  const omm::SplineType ease_up(omm::SplineType::Initialization::Ease, false);
  const omm::SplineType ease_down(omm::SplineType::Initialization::Ease, true);
  const omm::SplineType valley(omm::SplineType::Initialization::Valley, false);
  const omm::SplineType mountain(omm::SplineType::Initialization::Valley, true);
  const omm::SplineType center({{ 0.5, omm::SplineType::Knot(0.7, 0.0, 0.0) }});
  const omm::SplineType left({{ 0.0, omm::SplineType::Knot(0.7, 0.0, 0.0) }});
  const omm::SplineType right({{ 1.0, omm::SplineType::Knot(0.7, 0.0, 0.0) }});
  const omm::SplineType empty;

  const std::set<const omm::SplineType*> rot_symmetric { &up, &down, &ease_up, &ease_down };
  const std::set<const omm::SplineType*> flip_symmetric { &valley, &mountain };

  static constexpr double abs_eps = 0.0001;

  EXPECT_NEAR(0.0, ease_up.evaluate(0.0).value(), abs_eps);
  EXPECT_NEAR(0.0, ease_up.evaluate(0.0).derivative(), abs_eps);
  EXPECT_NEAR(1.0, ease_up.evaluate(1.0).value(), abs_eps);
  EXPECT_NEAR(0.0, ease_up.evaluate(1.0).derivative(), abs_eps);
  EXPECT_NEAR(0.5, ease_up.evaluate(0.5).value(), abs_eps);

  EXPECT_NEAR(1.0, ease_down.evaluate(0.0).value(), abs_eps);
  EXPECT_NEAR(0.0, ease_down.evaluate(0.0).derivative(), abs_eps);
  EXPECT_NEAR(0.0, ease_down.evaluate(1.0).value(), abs_eps);
  EXPECT_NEAR(0.0, ease_down.evaluate(1.0).derivative(), abs_eps);
  EXPECT_NEAR(0.5, ease_down.evaluate(0.5).value(), abs_eps);

  EXPECT_NEAR(0.0, mountain.evaluate(0.0).value(), abs_eps);
  EXPECT_NEAR(1.0, mountain.evaluate(0.5).value(), abs_eps);
  EXPECT_NEAR(0.0, mountain.evaluate(0.5).derivative(), abs_eps);
  EXPECT_NEAR(0.0, mountain.evaluate(1.0).value(), abs_eps);

  EXPECT_NEAR(1.0, valley.evaluate(0.0).value(), abs_eps);
  EXPECT_NEAR(0.0, valley.evaluate(0.5).value(), abs_eps);
  EXPECT_NEAR(0.0, valley.evaluate(0.5).derivative(), abs_eps);
  EXPECT_NEAR(1.0, valley.evaluate(1.0).value(), abs_eps);

  static constexpr std::size_t n = 10;
  for (std::size_t i = 0; i <= n; ++i) {
    double t = static_cast<double>(i) / static_cast<double>(n);
    for (const auto* s : rot_symmetric) {
      EXPECT_NEAR(s->evaluate(t).value(), 1.0 - s->evaluate(1.0-t).value(), abs_eps);
      EXPECT_NEAR(s->evaluate(t).derivative(), s->evaluate(1.0-t).derivative(), abs_eps);
    }
    for (const auto* s : flip_symmetric) {
      EXPECT_NEAR(s->evaluate(t).value(), s->evaluate(1.0-t).value(), abs_eps);
      EXPECT_NEAR(s->evaluate(t).derivative(), -s->evaluate(1.0-t).derivative(), abs_eps);
    }

    EXPECT_NEAR(up.evaluate(t).value(), t, abs_eps);
    EXPECT_NEAR(up.evaluate(t).derivative(), 1.0, abs_eps);

    EXPECT_NEAR(down.evaluate(t).value(), 1.0-t, abs_eps);
    EXPECT_NEAR(down.evaluate(t).derivative(), -1.0, abs_eps);

    EXPECT_NEAR(center.evaluate(t).value(), 0.7, abs_eps);
    EXPECT_NEAR(center.evaluate(t).derivative(), 0.0, abs_eps);

    EXPECT_NEAR(left.evaluate(t).value(), 0.7, abs_eps);
    EXPECT_NEAR(left.evaluate(t).derivative(), 0.0, abs_eps);

    EXPECT_NEAR(right.evaluate(t).value(), 0.7, abs_eps);
    EXPECT_NEAR(right.evaluate(t).derivative(), 0.0, abs_eps);

    EXPECT_NEAR(empty.evaluate(t).value(), 0.0, abs_eps);
    EXPECT_NEAR(empty.evaluate(t).derivative(), 0.0, abs_eps);
  }
}

