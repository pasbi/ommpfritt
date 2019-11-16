#include "managers/curvemanager/curvetimelinecanvas.h"
#include "preferences/uicolors.h"
#include "tools/handles/handle.h"
#include "properties/property.h"
#include "preferences/uicolors.h"

namespace
{

QPen make_pen(omm::HandleStatus status, const QString& type, std::size_t channel)
{
  const QString name = QString("%1-%2-fcurve").arg(type).arg(int(channel));
  const QColor color = omm::ui_color(status, "TimeLine", name);
  QPen pen;
  pen.setColor(color);
  pen.setWidth(2);
  return pen;
}

}  // namespace

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

void CurveTimelineCanvas
::draw_segment(QPainter& painter, Track& track, int kf1, int kf2, std::size_t i) const
{
  painter.drawLine( frame_range.unit_to_pixel(kf1),
                    value_range.unit_to_pixel(get_channel_value(track.knot_at(kf1).value, i)),
                    frame_range.unit_to_pixel(kf2),
                    value_range.unit_to_pixel(get_channel_value(track.knot_at(kf2).value, i)) );
}

void CurveTimelineCanvas
::draw_keyframe(QPainter& painter, Track& track, int keyframe, std::size_t i) const
{
  const QPointF p(frame_range.unit_to_pixel(keyframe),
                  value_range.unit_to_pixel(get_channel_value(track.knot_at(keyframe).value, i)));

  static const QPointF r(2, 2);
  painter.fillRect(QRectF(p - r, p + r), Qt::red);
}

void CurveTimelineCanvas::draw_keyframes(QPainter& painter) const
{
  painter.save();
  painter.translate(rect.topLeft());
  for (Track* track : tracks) {
    const std::size_t n = n_channels(track->property().variant_value());
    const std::vector<int> key_frames = track->key_frames();
    if (key_frames.size() > 0) {
      const int front = key_frames.front();
      const int back = key_frames.back();
      for (std::size_t i = 0; i < n; ++i) {
        painter.setPen(make_pen(HandleStatus::Inactive, track->type(), i));
        if (front >= frame_range.begin) {
          const double end = std::min<double>(frame_range.end, front);
          const double y = value_range.unit_to_pixel(get_channel_value(track->knot_at(front).value, i));
          painter.drawLine(frame_range.unit_to_pixel(frame_range.begin), y,
                           frame_range.unit_to_pixel(end), y);
        }
        if (back <= frame_range.end) {
          const double begin = std::max<double>(frame_range.begin, back);
          const double y = value_range.unit_to_pixel(get_channel_value(track->knot_at(back).value, i));
          painter.drawLine(frame_range.unit_to_pixel(begin), y,
                           frame_range.unit_to_pixel(frame_range.end), y);
        }

        const int k1 = key_frames[0];
        if (frame_range.begin <= k1 && k1 <= frame_range.end) {
          draw_keyframe(painter, *track, key_frames[0], i);
        }
        for (std::size_t k = 1; k < key_frames.size(); ++k) {
          const int k1 = key_frames[k-1];
          const int k2 = key_frames[k];
          if (k2 >= frame_range.begin || k1 <= frame_range.end) {
            draw_segment(painter, *track, k1, k2, i);
          }
          if (frame_range.begin <= k2 && k2 <= frame_range.end) {
            draw_keyframe(painter, *track, k2, i);
          }
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

}  // namespace omm
