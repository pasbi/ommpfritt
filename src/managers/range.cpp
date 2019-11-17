#include "managers/range.h"
#include <logging.h>
#include <list>
#include <cassert>
#include <cmath>
#include <algorithm>

namespace omm
{

double Range::pixel_to_unit(double pixel) const
{
  return normalized_to_unit(pixel / pixel_range());
}

double Range::unit_to_pixel(double unit) const
{
  return unit_to_normalized(unit) * pixel_range();
}

double Range::unit_to_normalized(double unit) const
{
  return (unit - begin) / (end - begin + 0);
}

double Range::normalized_to_unit(double normalized) const
{
  return normalized * (end - begin + 0) + begin;
}

std::vector<double> Range::scale(double distance) const
{
  assert(distance >= 0.0);
  const double spacing = this->spacing(distance);
  double v = begin - std::fmod(begin, spacing);
  std::list<double> scale;
  while (v <= end) {
    scale.push_back(v);
    v += spacing;
  }
  return std::vector(scale.begin(), scale.end());
}

double Range::spacing(double distance) const
{
  static constexpr auto steps = { 5.0, 2.0, 1.0 };
  const double spacing = distance / pixel_range() * (end - begin);
  const double m = std::pow(10.0, std::floor(std::log10(spacing)));
  const double f = [f=spacing/m]() {
    for (double s : steps) {
      if (f > s) {
        return s;
      }
    }
    return *std::prev(steps.end());
  }();
  return f * m;
}

void Range::pan(double d)
{
  const double b = normalized_to_unit(unit_to_normalized(begin) + d);
  const double e = normalized_to_unit(unit_to_normalized(end) + d);
  begin = b;
  end = e;
}

void Range::zoom(double origin, double amount, double min_upp, double max_upp)
{
  const double center = pixel_to_unit(origin);
  const double left = center - begin;
  const double right = center - end;

  double a = std::exp(amount / 300.0);
  const double upp = (left-right)*a / pixel_range();
  if (upp > max_upp) {
    a *= max_upp / upp;
  } else if (upp < min_upp) {
    a *= min_upp / upp;
  }
  begin = center - left * a;
  end = center - right * a;
}

}  // namespace omm
