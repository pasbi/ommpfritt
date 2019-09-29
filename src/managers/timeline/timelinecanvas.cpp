#include "managers/timeline/timelinecanvas.h"

#include "animation/track.h"
#include "animation/animator.h"
#include <QCursor>
#include <QMouseEvent>
#include "logging.h"

namespace omm
{

TimelineCanvas::TimelineCanvas(Animator& animator)
  : m_animator(animator)
  , m_draw_text(false)
{
}

void TimelineCanvas::set_font(const QFont& font)
{
  m_font = font;
  m_draw_text = true;
}

void TimelineCanvas::draw(QPainter& painter) const
{
  painter.save();
  painter.translate(rect.topLeft());
  painter.scale(rect.width(), rect.height());
  draw_background(painter);
  draw_lines(painter);
  draw_keyframes(painter);
  draw_current(painter);
  painter.restore();
}

void TimelineCanvas::draw_background(QPainter& painter) const
{
  painter.save();
  const double left = (m_animator.start()-left_frame) * ppf() - ppf()/2.0;
  const double right = (m_animator.end()-left_frame) * ppf() + ppf()/2.0;
  if (left > 0.0) {
    painter.fillRect(QRectF(QPointF(0, 0), QPointF(left, 1.0)), Qt::gray);
  }
  if (right < 1.0) {
    painter.fillRect(QRectF(QPointF(right, 0), QPointF(1.0, 1.0)), Qt::gray);
  }
  if (right > 0.0 && left < 1.0) {
    painter.fillRect(QRectF(QPointF(left, 0.0), QPointF(right, 1.0)), Qt::white);
  }
  painter.restore();
}

void TimelineCanvas::draw_lines(QPainter& painter) const
{
  painter.save();
  QPen pen;
  pen.setColor(Qt::black);
  pen.setCosmetic(true);
  painter.setPen(pen);

  if (m_draw_text) {
    painter.setFont(m_font);
  }
  for (int frame = left_frame; frame <= right_frame + 1; ++frame) {
    if (ppfs() < 10 && (frame % 2 != 0)) {
      continue;
    } else if (ppfs() < 2 && frame % 10 != 0) {
      continue;
    } else if (ppfs() < 1 && frame % 20 != 0) {
      continue;
    }

    const auto draw_frame_number = [this](int frame) {
      if (ppfs() < 2) {
        return frame % 100 == 0;
      } else if (ppfs() < 10) {
        return frame % 20 == 0;
      } else if (ppfs() < 20) {
        return frame % 10 == 0;
      } else {
        return frame % 2 == 0;
      }
    };

    pen.setWidthF(frame % 10 == 0 ? 2.0 : 1.0);
    painter.setPen(pen);

    const double x = (frame - left_frame) * ppf();
    const double line_start = frame % 2 == 0 ? 0 : 0.05;

    // there is no way in drawing really tiny text. Hence, we must draw the frame numbers in
    // non-normalized coordinates...
    painter.save();
    painter.resetTransform();
    painter.translate(rect.left() + rect.width() * x, rect.top());

    const double line_end = m_draw_text ? std::max(line_start, rect.height() - QFontMetricsF(m_font).height())
                                      : rect.height();
    painter.drawLine(QPointF(0, line_start), QPointF(0, line_end));

    if (m_draw_text && draw_frame_number(frame)) {
      const QString text = QString("%1").arg(frame);
      const double text_width = QFontMetrics(m_font).horizontalAdvance(text);
      painter.drawText(QPointF(-text_width/2.0, rect.height()), text);
    }
    painter.restore();
  }

  painter.restore();
}

void TimelineCanvas::draw_keyframes(QPainter& painter) const
{
  painter.save();
  painter.resetTransform();
  painter.translate(rect.topLeft());
  const int height = rect.height() - (m_draw_text ? QFontMetricsF(m_font).height() : 0);
  const int y = height / 2.0;
  QPainterPath diamond;
  diamond.moveTo(QPointF( 0,  1));
  diamond.lineTo(QPointF( 1,  0));
  diamond.lineTo(QPointF( 0, -1));
  diamond.lineTo(QPointF(-1,  0));
  diamond.closeSubpath();

  QPen pen;
  pen.setCosmetic(true);
  pen.setColor(Qt::black);
  const double scale = std::min(height/2.0, std::max(4.0, ppf())) * 2.0;
  pen.setWidthF(std::max(0.0, scale/5.0));
  painter.setPen(pen);

  for (int frame = left_frame; frame <= right_frame + 1; ++frame) {
    const bool draw = std::any_of(tracks.begin(), tracks.end(), [frame](const Track* track) {\
      return track->has_keyframe(frame);
    });
    if (draw) {
      painter.save();
      painter.translate((frame - left_frame) * ppfs(), y);
      painter.scale(scale, scale);
      painter.fillPath(diamond, Qt::yellow);
      painter.drawPath(diamond);
      painter.restore();
    }
  }
  painter.restore();
}

void TimelineCanvas::draw_current(QPainter& painter) const
{
  const double x = (m_animator.current()-left_frame) * ppf();
  const QRectF current_rect(QPointF(x-ppf()/2.0, 0.5), QSizeF(ppf(), 0.5));
  painter.fillRect(current_rect, QColor(255, 128, 0, 60));
  QPen pen;
  pen.setColor(QColor(255, 128, 0, 120));
  pen.setWidthF(4.0);
  pen.setCosmetic(true);
  painter.setPen(pen);
  painter.drawRect(current_rect);
}

double TimelineCanvas::ppf() const
{
  return 1.0 / (right_frame - left_frame + 1);
}

double TimelineCanvas::ppfs() const
{
  return rect.width() * ppf();
}

void TimelineCanvas::mouse_press(QMouseEvent& event)
{
  m_mouse_down_pos = event.pos();
  m_last_mouse_pos = event.pos();
  m_pan_active = event.modifiers() & Qt::AltModifier && event.button() == Qt::LeftButton;
  m_zoom_active = event.modifiers() & Qt::AltModifier && event.button() == Qt::RightButton;
  if (!m_pan_active && !m_zoom_active) {
    Q_EMIT current_frame_changed(std::round(event.pos().x() / (ppf() * rect.width()) + left_frame));
  }
}

void TimelineCanvas::mouse_move(QMouseEvent& event)
{
  const double min_ppf = 0.5 / rect.width();
  const double max_ppf = 70 / rect.width();
  const QPointF d = QPointF(m_last_mouse_pos - event.pos()) / rect.width();
  if (m_pan_active) {
    QCursor::setPos(map_to_global(m_mouse_down_pos));
    const double min = pixel_to_frame(frame_to_pixel(left_frame) + d.x());
    const double max = pixel_to_frame(frame_to_pixel(right_frame) + d.x());
    left_frame = min;
    right_frame = max;
    update();
  } else if (m_zoom_active) {
    QCursor::setPos(map_to_global(m_mouse_down_pos));
    const double left = (m_mouse_down_pos.x() - rect.left()) / rect.width();
    const double center_frame = pixel_to_frame(left);
    const double right = 1.0 - left;;
    double ppf = this->ppf() * std::exp(-d.x() * rect.width() / 300.0);

    ppf = std::clamp(ppf, min_ppf, max_ppf);
    left_frame = center_frame - left / ppf;
    right_frame = center_frame + right  / ppf - 1.0;
    update();
  } else {
    Q_EMIT current_frame_changed(std::round(event.pos().x() / (ppf() * rect.width()) + left_frame));
  }
}

void TimelineCanvas::mouse_release(QMouseEvent& event)
{
  Q_UNUSED(event)
  m_pan_active = false;
}

double TimelineCanvas::pixel_to_frame(double pixel) const
{
  return pixel / ppf() + left_frame;
}

double TimelineCanvas::frame_to_pixel(double frame) const
{
  return (frame - left_frame) * ppf();
}

}  // namespace omm
