#include "properties/numericproperty.h"

namespace
{
  static constexpr int int_high = std::numeric_limits<int>::max();
  static constexpr int int_low = std::numeric_limits<int>::lowest();
  static constexpr double inf = std::numeric_limits<double>::infinity();
}  // namespace

namespace omm
{

template<> const double NumericProperty<double>::highest_possible_value(inf);
template<> const double NumericProperty<double>::lowest_possible_value(-inf);
template<> const double NumericProperty<double>::smallest_step(0.001);
template<> const double NumericProperty<double>::default_step(1.0);

template<> const int NumericProperty<int>::highest_possible_value(int_high);
template<> const int NumericProperty<int>::lowest_possible_value(int_low);
template<> const int NumericProperty<int>::smallest_step = 1;
template<> const int NumericProperty<int>::default_step(1);

template<> const Vec2i NumericProperty<Vec2i>::highest_possible_value(int_high, int_high);
template<> const Vec2i NumericProperty<Vec2i>::lowest_possible_value(int_low, int_low);
template<> const Vec2i NumericProperty<Vec2i>::smallest_step(1, 1);
template<> const Vec2i NumericProperty<Vec2i>::default_step(1, 1);


template<> const Vec2f NumericProperty<Vec2f>::highest_possible_value(inf, inf);
template<> const Vec2f NumericProperty<Vec2f>::lowest_possible_value(-inf, -inf);
template<> const Vec2f NumericProperty<Vec2f>::smallest_step(1.0, 1.0);
template<> const Vec2f NumericProperty<Vec2f>::default_step(1.0, 1.0);

}  // namespace omm
