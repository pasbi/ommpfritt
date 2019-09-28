#include "managers/timeline/slider.h"
#include "logging.h"
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
#include "animation/animator.h"
#include "scene/scene.h"
#include "scene/messagebox.h"

namespace omm
{

Slider::Slider(Animator& animator)
  : animator(animator)
  , m_min(1)
  , m_max(100)
{
  connect(&animator, SIGNAL(start_changed(int)), this, SLOT(update()));
  connect(&animator, SIGNAL(end_changed(int)), this, SLOT(update()));
  connect(&animator, SIGNAL(current_changed(int)), this, SLOT(update()));
  connect(&animator, SIGNAL(track_changed(Track&)), this, SLOT(update_keyframe_hints()));
  connect(&animator.scene.message_box(),
          SIGNAL(selection_changed(const std::set<AbstractPropertyOwner*>&)),\
          this, SLOT(update_keyframe_hints()));
}

void Slider::draw_background(const Animator& animator, QPainter& painter,
                             double left_frame, double right_frame)
{
  painter.save();
  const double ppf = 1.0 / (right_frame - left_frame + 1);
  const double left = (animator.start()-left_frame) * ppf - ppf/2.0;
  const double right = (animator.end()-left_frame) * ppf + ppf/2.0;
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

void Slider::draw_lines(QPainter& painter, double left_frame, double right_frame, const QFont& font,
                        const QRectF& rect, bool draw_text)
{
  painter.save();
  const double ppf = 1.0 / (right_frame - left_frame + 1);
  const double ppfs = rect.width() * ppf;

  QFont f(font);
  painter.setFont(f);
  const QFontMetricsF fm(f);
  QPen pen;
  pen.setColor(Qt::black);
  pen.setCosmetic(true);
  painter.setPen(pen);
  for (int frame = left_frame; frame <= right_frame + 1; ++frame) {
    if (ppfs < 10 && (frame % 2 != 0)) {
      continue;
    } else if (ppfs < 2 && frame % 10 != 0) {
      continue;
    } else if (ppfs < 1 && frame % 20 != 0) {
      continue;
    }

    bool draw_frame_number = draw_text;
    if (ppfs < 2) {
      draw_frame_number &= frame % 100 == 0;
    } else if (ppfs < 10) {
      draw_frame_number &= frame % 20 == 0;
    } else if (ppfs < 20) {
      draw_frame_number &= frame % 10 == 0;
    } else {
      draw_frame_number &= frame % 2 == 0;
    }

    pen.setWidthF(frame % 10 == 0 ? 2.0 : 1.0);
    painter.setPen(pen);

    const double x = (frame - left_frame) * ppf;
    const double line_start = frame % 2 == 0 ? 0 : 0.05;

    // there is no way in drawing really tiny text. Hence, we must draw the frame numbers in
    // non-normalized coordinates...
    painter.save();
    painter.resetTransform();
    painter.translate(rect.left() + rect.width() * x, rect.top());

    const double line_end = draw_text ? std::max(line_start, rect.height() - fm.height())
                                      : rect.height();
    painter.drawLine(QPointF(0, line_start), QPointF(0, line_end));

    if (draw_frame_number) {
      const QString text = QString("%1").arg(frame);
      painter.drawText(QPointF(-fm.horizontalAdvance(text)/2.0, rect.height()), text);
    }
    painter.restore();
  }

  painter.restore();
}

void Slider::set_min(double frame)
{
  m_min = frame;
  update();
}

void Slider::set_max(double frame)
{
  m_max = frame;
  update();
}

void Slider::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);

  painter.translate(rect().topLeft());
  painter.scale(rect().width(), rect().height());

  painter.setRenderHint(QPainter::HighQualityAntialiasing);

  draw_background(animator, painter, m_min, m_max);

  painter.save();
  draw_lines(painter, m_min, m_max, font(), rect(), true);
  painter.restore();

  painter.save();
  draw_keyframe_hints(painter, m_min, m_max, font(), rect(), m_keyframe_hints);
  painter.restore();

  painter.save();
  draw_current(animator, painter, m_min, m_max);
  painter.restore();

  QWidget::paintEvent(event);
}

void Slider::mousePressEvent(QMouseEvent *event)
{
  m_mouse_down_pos = event->pos();
  m_last_mouse_pos = event->pos();
  m_pan_active = event->modifiers() & Qt::AltModifier && event->button() == Qt::LeftButton;
  m_zoom_active = event->modifiers() & Qt::AltModifier && event->button() == Qt::RightButton;
  if (!m_pan_active && !m_zoom_active) {
    Q_EMIT value_changed(pixel_to_frame(event->pos().x()) + 0.5);
  }
  QWidget::mousePressEvent(event);
}

void Slider::mouseMoveEvent(QMouseEvent *event)
{
  static constexpr double min_ppf = 0.5;
  static constexpr double max_ppf = 70;
  if (m_pan_active) {
    const QPoint d = m_last_mouse_pos - event->pos();
    QCursor::setPos(mapToGlobal(m_mouse_down_pos));
    const double min = pixel_to_frame(frame_to_pixel(m_min) + d.x());
    const double max = pixel_to_frame(frame_to_pixel(m_max) + d.x());
    m_min = min;
    m_max = max;
    update();
  } else if (m_zoom_active) {
    const QPoint d = m_last_mouse_pos - event->pos();
    QCursor::setPos(mapToGlobal(m_mouse_down_pos));
    const double center_frame = pixel_to_frame(m_mouse_down_pos.x());
    const int left = m_mouse_down_pos.x();
    const int right = width() - m_mouse_down_pos.x();
    double ppf = pixel_per_frame() * std::exp(-d.x() / 300.0);
    ppf = std::clamp(ppf, min_ppf, max_ppf);
    m_min = center_frame - left / ppf;
    m_max = center_frame + right  / ppf - 1;
    update();
  } else {
    Q_EMIT value_changed(pixel_to_frame(event->pos().x()) + 0.5);
  }
  QWidget::mouseMoveEvent(event);
}

void Slider::mouseReleaseEvent(QMouseEvent* event)
{
  m_pan_active = false;
  QWidget::mouseReleaseEvent(event);
}

double Slider::frame_to_pixel(double frame) const
{
  return (frame - m_min) * pixel_per_frame();
}

double Slider::pixel_to_frame(double pixel) const
{
  return pixel / pixel_per_frame() + m_min;
}

double Slider::pixel_per_frame() const
{
  return width() / static_cast<double>(m_max - m_min + 1);
}

void Slider::update_keyframe_hints()
{
  m_keyframe_hints.clear();
  for (const AbstractPropertyOwner* o : animator.scene.selection()) {
    for (Property* property : o->properties().values()) {
      if (Track* track = property->track(); track != nullptr) {
        const auto key_frames = track->key_frames();
        m_keyframe_hints.insert(key_frames.begin(), key_frames.end());
      }
    }
  }
  update();
}

void Slider::draw_current(Animator& animator, QPainter& painter, double left_frame, double right_frame)
{
  const double ppf = 1.0 / (right_frame - left_frame + 1);
  const double x = (animator.current()-left_frame) * ppf;
  const QRectF current_rect(QPointF(x-ppf/2.0, 0.5), QSizeF(ppf, 0.5));
  painter.fillRect(current_rect, QColor(255, 128, 0, 60));
  QPen pen;
  pen.setColor(QColor(255, 128, 0, 120));
  pen.setWidthF(4.0);
  pen.setCosmetic(true);
  painter.setPen(pen);
  painter.drawRect(current_rect);
}

void Slider::draw_keyframe_hints(QPainter& painter, double left_frame, double right_frame,
                                 const QFont& font, const QRectF& rect, const std::set<int>& hints)
{
  painter.save();
  painter.resetTransform();
  painter.translate(rect.topLeft());
  const double ppf = rect.width() / (right_frame - left_frame + 1);
  const QFontMetricsF fm(font);
  const int height = (rect.height() - fm.height());
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
  const double scale = std::min(height/2.0, std::max(4.0, ppf))/2.0;
  pen.setWidthF(scale/5.0);
  painter.setPen(pen);

  for (int frame = left_frame; frame <= right_frame + 1; ++frame) {
    if (::contains(hints, frame)) {
      painter.save();
      painter.translate((frame - left_frame) * ppf, y);
      painter.scale(scale, scale);
      painter.fillPath(diamond, Qt::yellow);
      painter.drawPath(diamond);
      painter.restore();
    }
  }
  painter.restore();
}

}  // namespace omm
