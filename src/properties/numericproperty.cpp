#include "properties/numericproperty.h"

namespace omm
{
template<> double NumericProperty<double>::highest_possible_value()
{
  return std::numeric_limits<double>::infinity();
}

template<> double NumericProperty<double>::lowest_possible_value()
{
  return -std::numeric_limits<double>::infinity();
}

template<> double NumericProperty<double>::smallest_step()
{
  static constexpr double SMALLEST_STEP = 0.001;
  return SMALLEST_STEP;
}

template<> double NumericProperty<double>::default_step()
{
  return 1.0;
}

template<> int NumericProperty<int>::highest_possible_value()
{
  return std::numeric_limits<int>::max();
}

template<> int NumericProperty<int>::lowest_possible_value()
{
  return std::numeric_limits<int>::min();
}

template<> int NumericProperty<int>::smallest_step()
{
  return 1;
}

template<> int NumericProperty<int>::default_step()
{
  return 1;
}

template<> Vec2i NumericProperty<Vec2i>::highest_possible_value()
{
  return {NumericProperty<int>::highest_possible_value(),
          NumericProperty<int>::highest_possible_value()};
}

template<> Vec2i NumericProperty<Vec2i>::lowest_possible_value()
{
  return {NumericProperty<int>::lowest_possible_value(),
          NumericProperty<int>::lowest_possible_value()};
}

template<> Vec2i NumericProperty<Vec2i>::smallest_step()
{
  return {1, 1};
}

template<> Vec2i NumericProperty<Vec2i>::default_step()
{
  return {1, 1};
}

template<> Vec2f NumericProperty<Vec2f>::highest_possible_value()
{
  return {NumericProperty<double>::highest_possible_value(),
          NumericProperty<double>::highest_possible_value()};
}

template<> Vec2f NumericProperty<Vec2f>::lowest_possible_value()
{
  return {NumericProperty<double>::lowest_possible_value(),
          NumericProperty<double>::lowest_possible_value()};
}

template<> Vec2f NumericProperty<Vec2f>::smallest_step()
{
  return {1.0, 1.0};
}

template<> Vec2f NumericProperty<Vec2f>::default_step()
{
  return {1.0, 1.0};
}

template<> bool NumericProperty<bool>::highest_possible_value()
{
  return true;
}

template<> bool NumericProperty<bool>::lowest_possible_value()
{
  return false;
}

template<> bool NumericProperty<bool>::smallest_step()
{
  return true;
}

template<> bool NumericProperty<bool>::default_step()
{
  return true;
}

}  // namespace omm
