#include "splinetype.h"
#include <algorithm>

namespace
{

using namespace omm;

template<typename Knots> auto find_knot(Knots&& knots, double t)
{
  assert(std::is_sorted(knots.begin(), knots.end()));
  return std::find_if(knots.begin(), knots.end(), [t](const auto& knot) { return knot.t > t; });
}

template<typename Knots> auto* find_closest_knot(Knots&& knots, double t)
{
  const auto it = find_knot(knots, t);
  if (it == knots.end()) {
    return static_cast<decltype(&*knots.begin())>(nullptr);
  } else if (it == knots.begin()) {
    return &*it;
  } else {
    const auto prev = std::prev(it);
    return (std::abs(prev->t - t) < std::abs(it->t - t)) ? &*prev : &*it;
  }
}

}  // namespace

namespace omm
{

SplineType::ControlPoint SplineType::begin()
{
  return ControlPoint(knots, knots.begin(), Knot::Side::Middle);
}

SplineType::SplineType(const std::multimap<double, SplineType::Knot>& knots)
  : knots(knots)
{
}

bool SplineType::operator==(const SplineType& other) const
{
  return knots == other.knots;
}

bool SplineType::operator<(const SplineType& other) const
{
  return  knots < other.knots;
}

SplineType::knot_map_type::iterator SplineType::move(knot_map_type::const_iterator it, double new_t)
{
  assert(it != knots.end());
  auto next = std::next(it);
  auto node = knots.extract(it);
  node.key() = new_t;
  if (knots.empty()) {
    next = knots.begin();
  } else {
    next = std::prev(next);
  }
  return knots.insert(next, std::move(node));
}

SplineType::Knot::Knot(double value, double left_offset, double right_offset)
  : value(value), left_offset(left_offset), right_offset(right_offset)
{
}

bool SplineType::Knot::operator==(const SplineType::Knot& other) const
{
  return value == other.value
      && left_offset == other.left_offset
      && right_offset == other.right_offset;
}

bool SplineType::Knot::operator<(const SplineType::Knot& other) const
{
  const auto to_array = [](const SplineType::Knot& knot) {
    return std::array { knot.value, knot.left_offset, knot.right_offset };
  };

  return to_array(*this) < to_array(other);
}

double SplineType::Knot::get_value(const SplineType::Knot::Side side) const
{
  switch (side) {
  case Side::Left:
    return left_offset + value;
  case Side::Right:
    return right_offset + value;
  case Side::Middle:
    return value;
  default:
    Q_UNREACHABLE();
    return 0.0;
  }
}

void SplineType::Knot::set_value(const SplineType::Knot::Side side, double value)
{
  switch (side) {
  case Side::Left:
    left_offset = value - this->value;
    break;
  case Side::Right:
    right_offset = value - this->value;
    break;
  case Side::Middle:
    this->value = value;
    break;
  default:
    Q_UNREACHABLE();
  }
}

}  // namespace omm
