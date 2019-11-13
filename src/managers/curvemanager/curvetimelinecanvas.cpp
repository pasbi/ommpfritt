#include "managers/curvemanager/curvetimelinecanvas.h"
#include "preferences/uicolors.h"

namespace omm
{

CurveTimelineCanvas::CurveTimelineCanvas(Animator& animator, QWidget& widget)
  : TimelineCanvas(animator, widget), value_range(*this)
{
}

void CurveTimelineCanvas::draw_lines(QPainter& painter) const
{
  TimelineCanvas::draw_lines(painter);
  painter.save();
  QPen pen;
  pen.setColor(ui_color(m_widget, "TimeLine", "scale hline"));
  painter.setPen(pen);
  assert(value_range.begin < value_range.end);
  const double vspan = (value_range.end - value_range.begin) / value_range.pixel_range() * 1000.0;
  double logvspan = std::log10(vspan);
  logvspan -= std::fmod(logvspan, 1.0);
  const double spacing = std::pow(10.0, logvspan - 1);
  painter.drawText(QPointF(0.0, painter.fontMetrics().height()),
                   QString("%1").arg(value_range.begin));
  painter.drawText(QPointF(0.0, rect.height()),
                   QString("%1").arg(value_range.end));
  double y = value_range.begin - std::fmod(value_range.begin, spacing);
  while (y < value_range.end) {
    const double py = (y - value_range.begin) / (value_range.end - value_range.begin) * rect.height();
    const double x = rect.width();
    painter.drawLine(QPointF(0.0, py), QPointF(x, py));
    y += spacing;
  }
  painter.restore();
}

void CurveTimelineCanvas::draw_fcurve(QPainter& painter) const
{
  painter.save();
  painter.translate(rect.topLeft());
  for (Track* track : tracks) {
    if (track->type() == "Float") {
      for (int frame = frame_range.begin; frame <= frame_range.end; ++frame) {
        if (track->has_keyframe(frame)) {
          const double x = rect.width() * (frame - frame_range.begin) * frame_range.units_per_pixel();
          painter.drawLine(QPointF(x, 0), QPointF(x, rect.height()));
        }
      }
    }
  }
  painter.restore();
}

void CurveTimelineCanvas::pan(const QPointF& d)
{
  TimelineCanvas::pan(d);
  value_range.pan(d.y());
}

void CurveTimelineCanvas::zoom(const QPointF& d)
{
  if (std::abs(d.x()) > std::abs(d.y())) {
    TimelineCanvas::zoom(d);
  } else {
    value_range.zoom(m_mouse_down_pos.y(), d.y(), 0.0, 10000.0);
  }
}

double TimelineCanvas::Range::units_per_pixel() const
{
  return 1.0 / (end - begin + 1);
}

double TimelineCanvas::Range::pixel_to_unit(double pixel) const
{
  return pixel / (units_per_pixel() * pixel_range()) + begin;
}

double TimelineCanvas::Range::unit_to_pixel(double unit) const
{
  return (unit - begin) * units_per_pixel() * pixel_range();
}

double TimelineCanvas::Range::unit_to_normalized(double unit) const
{
  return (unit - begin) * units_per_pixel();
}

double TimelineCanvas::Range::normalized_to_unit(double normalized) const
{
  return normalized / units_per_pixel() + begin;
}

double TimelineCanvas::Range::width() const
{
  return pixel_range() * units_per_pixel();
}

void TimelineCanvas::Range::pan(double d)
{
  const double b = normalized_to_unit(unit_to_normalized(begin) + d);
  const double e = normalized_to_unit(unit_to_normalized(end) + d);
  begin = b;
  end = e;
}

void TimelineCanvas::Range::zoom(double origin, double amount, double min_upp, double max_upp)
{
  double b = (origin - unit_to_pixel(begin)) / pixel_range();
  double center = normalized_to_unit(b);
  double ppf = units_per_pixel() * std::exp(-amount * pixel_range() / 300.0);
  ppf = std::clamp(ppf, min_upp, max_upp);
  begin = center - b / ppf;
  end = center - (b - 1.0) / ppf - 1.0;
}

}  // namespace omm
