#include "splinetype.h"
#include "logging.h"
#include "serializers/deserializerworker.h"
#include "serializers/serializerworker.h"
#include <algorithm>
#include <cmath>

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

template<std::size_t N>
double convolve(const std::array<double, 4>& coefficients,
                const std::array<std::array<double, N>, 4>& b,
                double t)
{
  double value = 0.0;
  for (std::size_t i = 0; i < b.size(); ++i) {
    assert(b.at(i).size() == N);
    for (std::size_t j = 0; j < b.at(i).size(); ++j) {
      value += coefficients.at(i) * b.at(i).at(j) * std::pow(t, j);
    }
  }
  return value;
}

using Knot = omm::SplineType::Knot;
using SpInit = omm::SplineType::Initialization;

const std::map<SpInit, omm::SplineType::knot_map_type> predefined{
    {SpInit::Linear,
     {
         {0.0, Knot(0.0, {-1.0 / 3.0, 1.0 / 3.0})},
         {1.0, Knot(1.0, {-1.0 / 3.0, 1.0 / 3.0})},
     }},
    {SpInit::Ease,
     {
         {0.0, Knot(0.0, {0.0, 0.0})},
         {1.0, Knot(1.0, {0.0, 0.0})},
     }},
    {SpInit::Valley,
     {
         {0.0, Knot(1.0, {0.0, 0.0})},
         {0.5, Knot(0.0, {0.0, 0.0})},
         {1.0, Knot(1.0, {0.0, 0.0})},
     }},
};

}  // namespace

namespace omm
{

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
QString SplineType::to_string() const
{
  return {};
}

void SplineType::serialize(serialization::SerializerWorker& worker) const
{
  std::vector<std::vector<double>> values;
  values.reserve(knots.size());
  for (const auto& [t, knot] : knots) {
    values.push_back({t, knot.value, knot.left_offset, knot.right_offset});
  }
  worker.set_value(values);
}

void SplineType::deserialize(serialization::DeserializerWorker& worker)
{
  knots.clear();
  worker.get_items([this](auto& worker_i) {
    const auto values = worker_i.template get<std::vector<double>>();
    knots.emplace(values.at(0), SplineType::Knot(values.at(1), {values.at(2), values.at(3)}));
  });
}

SplineType::ControlPoint SplineType::begin()
{
  return ControlPoint(knots, knots.begin(), Knot::Side::Middle);
}

SplineType::SplineType(const std::multimap<double, SplineType::Knot>& knots) : knots(knots)
{
}

SplineType::SplineType(Initialization initialization, bool flip)
    : knots(predefined.at(initialization))
{
  if (flip) {
    for (auto& [t, knot] : knots) {
      knot.value = 1.0 - knot.value;
      knot.left_offset *= -1.0;
      knot.right_offset *= -1.0;
    }
  }
}

bool SplineType::operator==(const SplineType& other) const
{
  return knots == other.knots;
}

bool SplineType::operator!=(const SplineType& other) const
{
  return !(*this == other);
}

bool SplineType::operator<(const SplineType& other) const
{
  return knots < other.knots;
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

SplineType::Interpolation SplineType::evaluate(double t) const
{
  const auto lower = knots.lower_bound(t);
  Interpolation interpolation;
  interpolation.t = t;
  if (knots.empty()) {
    // keep left and right default.
  } else if (lower == knots.end()) {
    if (knots.size() > 1 && t == std::prev(lower)->first) {
      interpolation.left = *std::prev(lower, 2);
      interpolation.right = *std::prev(lower);
    } else {
      interpolation.left = *std::prev(lower);
    }
  } else if (lower == knots.begin()) {
    if (knots.size() > 1 && t == lower->first) {
      interpolation.left = *lower;
      interpolation.right = *std::next(lower);
    } else {
      interpolation.right = *lower;
    }
  } else {
    interpolation.right = *lower;
    interpolation.left = *std::prev(lower);
  }
  return interpolation;
}

SplineType::Knot::Knot(double value, const std::pair<double, double>& left_right_offset)
    : value(value), left_offset(left_right_offset.first), right_offset(left_right_offset.second)
{
}

bool SplineType::Knot::operator==(const SplineType::Knot& other) const
{
  return value == other.value && left_offset == other.left_offset
         && right_offset == other.right_offset;
}

bool Knot::operator!=(const Knot& other) const
{
  return !(*this == other);
}

bool SplineType::Knot::operator<(const SplineType::Knot& other) const
{
  const auto to_array = [](const SplineType::Knot& knot) {
    return std::array{knot.value, knot.left_offset, knot.right_offset};
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

double SplineType::Interpolation::local_t() const
{
  const double left = this->left.has_value() ? this->left->first : 0.0;
  const double right = this->right.has_value() ? this->right->first : 1.0;
  assert(left <= right);
  static constexpr double eps = 0.00001;
  if (abs(left - right) < eps) {
    return 0.0;
  } else {
    return (t - left) / (right - left);
  }
}

double SplineType::Interpolation::value() const
{
  static constexpr std::array<std::array<double, 4>, 4> b{
      std::array<double, 4>{1.0, -3.0, 3.0, -1.0},
      std::array<double, 4>{0.0, 3.0, -6.0, 3.0},
      std::array<double, 4>{0.0, 0.0, 3.0, -3.0},
      std::array<double, 4>{0.0, 0.0, 0.0, 1.0},
  };
  return convolve(coefficients(), b, local_t());
}

double SplineType::Interpolation::derivative() const
{
  static constexpr std::array<std::array<double, 3>, 4> b{
      std::array<double, 3>{-3.0, 6.0, -3.0},
      std::array<double, 3>{3.0, -12.0, 9.0},
      std::array<double, 3>{0.0, 6.0, -9.0},
      std::array<double, 3>{0.0, 0.0, 3.0},
  };
  return convolve(coefficients(), b, local_t());
}

std::array<double, 4> SplineType::Interpolation::coefficients() const
{
  using Side = SplineType::Knot::Side;
  if (left.has_value() && right.has_value()) {
    return {left->second.get_value(Side::Middle),
            left->second.get_value(Side::Right),
            right->second.get_value(Side::Left),
            right->second.get_value(Side::Middle)};
  } else if (left.has_value()) {
    const double v = left->second.get_value(Side::Middle);
    return {v, v, v, v};
  } else if (right.has_value()) {
    const double v = right->second.get_value(Side::Middle);
    return {v, v, v, v};
  } else {
    return {0.0, 0.0, 0.0, 0.0};
  }
}

}  // namespace omm
