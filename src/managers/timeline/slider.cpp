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
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
  const int left = frame_to_pixel(animator.start()) - pixel_per_frame()/2.0;
  const int right = frame_to_pixel(animator.end()) + pixel_per_frame()/2.0;
  if (left > 0) {
    painter.fillRect(QRect(QPoint(0, 0), QPoint(left-1, height())), Qt::gray);
  }
  if (right < width()) {
    painter.fillRect(QRect(QPoint(right+1, 0), QPoint(width(), height())), Qt::gray);
  }
  if (right > 0 && left < width()) {
    painter.fillRect(QRect(QPoint(left, 0), QPoint(right, height())), Qt::white);
  }

  painter.save();
  draw_lines(painter);
  painter.restore();

  painter.save();
  draw_keyframe_hints(painter);
  painter.restore();

  painter.save();
  painter.translate(frame_to_pixel(animator.current()), 0);
  draw_current(painter);
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

void Slider::draw_lines(QPainter& painter) const
{
  const QFontMetricsF fm(font());
  const double ppf = this->pixel_per_frame();
  QPen pen;
  pen.setColor(Qt::black);
  pen.setCosmetic(true);
  painter.setPen(pen);
  for (int frame = m_min; frame <= m_max + 1; ++frame) {
    if (ppf < 10 && (frame % 2 != 0)) {
      continue;
    } else if (ppf < 2 && frame % 10 != 0) {
      continue;
    } else if (ppf < 1 && frame % 20 != 0) {
      continue;
    }

    bool draw_frame_number = false;
    if (ppf < 2) {
      draw_frame_number = frame % 100 == 0;
    } else if (ppf < 10) {
      draw_frame_number = frame % 20 == 0;
    } else if (ppf < 20) {
      draw_frame_number = frame % 10 == 0;
    } else {
      draw_frame_number = frame % 2 == 0;
    }

    pen.setWidthF(frame % 10 == 0 ? 2.0 : 1.0);
    painter.setPen(pen);
    const double x = frame_to_pixel(frame);
    const int line_start = frame % 2 == 0 ? 0 : 5;
    const int line_end = std::max(line_start, static_cast<int>(height() - fm.height()));
    painter.drawLine(x, line_start, x, line_end);

    if (draw_frame_number) {
      const QString text = QString("%1").arg(frame);
      painter.drawText(QPointF(x - fm.horizontalAdvance(text)/2.0, height()), text);
    }
  }
}

void Slider::draw_current(QPainter& painter) const
{
  const double height = this->height();
  const double pixel_per_frame = this->pixel_per_frame();
  const QRectF current_rect(-pixel_per_frame/2.0, height/2.0, pixel_per_frame, height);
  painter.fillRect(current_rect, QColor(255, 128, 0, 60));
  QPen pen;
  pen.setColor(QColor(255, 128, 0, 120));
  pen.setWidthF(4.0);
  painter.setPen(pen);
  painter.drawRect(current_rect);
}

void Slider::draw_keyframe_hints(QPainter& painter) const
{
  const QFontMetricsF fm(font());
  const double ppf = this->pixel_per_frame();
  const int height = (this->height() - fm.height());
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

  for (int frame = m_min; frame <= m_max + 1; ++frame) {
    if (::contains(m_keyframe_hints, frame)) {
      painter.save();
      painter.translate(frame_to_pixel(frame), y);
      painter.scale(scale, scale);
      painter.fillPath(diamond, Qt::yellow);
      painter.drawPath(diamond);
      painter.restore();
    }
  }
}

}  // namespace omm
