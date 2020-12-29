#include "managers/range.h"
#include "logging.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <list>

namespace omm
{
Range::Range(double begin, double end, Range::Options options)
    : begin(begin), end(end), mirror(!!(options & Options::Mirror))
{
}

double Range::pixel_to_unit(double pixel) const
{
  if (mirror) {
    pixel = pixel_range() - pixel;
  }
  return normalized_to_unit(pixel / pixel_range());
}

double Range::unit_to_pixel(double unit) const
{
  double pixel = unit_to_normalized(unit) * pixel_range();
  if (mirror) {
    pixel = pixel_range() - pixel;
  }
  return pixel;
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
  static constexpr auto steps = {5.0, 2.0, 1.0};
  const double spacing = distance / pixel_range() * (end - begin);
  const double m = std::pow(10.0, std::floor(std::log10(spacing)));
  const double f = [f = spacing / m]() {
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
  if (mirror) {
    d *= -1.0;
  }
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

  const double ZOOM_SPEED = 1.0 / 300.0;

  double a = std::exp(amount * ZOOM_SPEED);
  const double upp = (left - right) * a / pixel_range();
  if (upp > max_upp) {
    a *= max_upp / upp;
  } else if (upp < min_upp) {
    a *= min_upp / upp;
  }
  begin = center - left * a;
  end = center - right * a;
}

WidgetRange2D::WidgetRange2D(QPointF begin,
                             QPointF end,
                             QWidget& widget,
                             Range::Options h_options,
                             Range::Options v_options)
    : h_range(begin.x(), end.x(), widget, h_options), v_range(begin.y(), end.y(), widget, v_options)
{
}

QPointF WidgetRange2D::begin() const
{
  return {h_range.begin, v_range.begin};
}
void WidgetRange2D::set_begin(const QPointF& begin)
{
  h_range.begin = begin.x();
  v_range.begin = begin.y();
}

QPointF WidgetRange2D::end() const
{
  return {h_range.end, v_range.end};
}
void WidgetRange2D::set_end(const QPointF& end)
{
  h_range.end = end.x();
  v_range.end = end.y();
}

QPointF WidgetRange2D::pixel_to_unit(const QPointF& pixel) const
{
  return {h_range.pixel_to_unit(pixel.x()), v_range.pixel_to_unit(pixel.y())};
}

QPointF WidgetRange2D::unit_to_pixel(const QPointF& unit) const
{
  return {h_range.unit_to_pixel(unit.x()), v_range.unit_to_pixel(unit.y())};
}

QPointF WidgetRange2D::unit_to_normalized(const QPointF& unit) const
{
  return {h_range.unit_to_normalized(unit.x()), v_range.unit_to_normalized(unit.y())};
}

QPointF WidgetRange2D::normalized_to_unit(const QPointF& normalized) const
{
  return {h_range.normalized_to_unit(normalized.x()), v_range.normalized_to_unit(normalized.y())};
}

void WidgetRange2D::pan(const QPointF& d)
{
  h_range.pan(d.x());
  v_range.pan(d.y());
}

void WidgetRange2D::zoom(const QPointF& origin,
                         const QPointF& amount,
                         const QPointF& min_upp,
                         const QPointF& max_upp)
{
  h_range.zoom(origin.x(), amount.x(), min_upp.x(), max_upp.x());
  v_range.zoom(origin.y(), amount.y(), min_upp.y(), max_upp.y());
}

}  // namespace omm
