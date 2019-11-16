#include "managers/range.h"
#include <cmath>
#include <algorithm>

namespace omm
{

double Range::units_per_pixel() const
{
  return 1.0 / (end - begin + 1);
}

double Range::pixel_to_unit(double pixel) const
{
  return pixel / (units_per_pixel() * pixel_range()) + begin;
}

double Range::unit_to_pixel(double unit) const
{
  return (unit - begin) * units_per_pixel() * pixel_range();
}

double Range::unit_to_normalized(double unit) const
{
  return (unit - begin) * units_per_pixel();
}

double Range::normalized_to_unit(double normalized) const
{
  return normalized / units_per_pixel() + begin;
}

double Range::width() const
{
  return pixel_range() * units_per_pixel();
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
  double b = (origin - unit_to_pixel(begin)) / pixel_range();
  double center = normalized_to_unit(b);
  double ppf = units_per_pixel() * std::exp(-amount * pixel_range() / 300.0);
  ppf = std::clamp(ppf, min_upp, max_upp);
  begin = center - b / ppf;
  end = center - (b - 1.0) / ppf - 1.0;
}

}  // namespace omm
