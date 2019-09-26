#include "managers/timeline/slider.h"
#include "logging.h"
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
#include "animation/animator.h"

namespace
{


}  // namespace

namespace omm
{

Slider::Slider(QWidget* parent) : QWidget(parent), m_min(1), m_max(100)
{
}

void Slider::set_animator(Animator& animator)
{
  connect(&animator, SIGNAL(start_changed(int)), this, SLOT(update()));
  connect(&animator, SIGNAL(end_changed(int)), this, SLOT(update()));
  connect(&animator, SIGNAL(current_changed(int)), this, SLOT(update()));
  m_animator = &animator;
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
  assert(m_animator != nullptr);
  QPainter painter(this);
  const int left = frame_to_pixel(m_animator->start()) - pixel_per_frame() * 0.5 + left_margin;
  const int right = frame_to_pixel(m_animator->end()) + pixel_per_frame() * 0.5 + left_margin;
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
  painter.translate(left_margin, bottom_margin);

  painter.save();
  draw_lines(painter);
  painter.restore();

  painter.save();
  painter.translate(frame_to_pixel(m_animator->current()), 0);
  draw_current(painter);
  painter.restore();

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
    Q_EMIT value_changed(pixel_to_frame(event->pos().x() - left_margin));
  }
  QWidget::mousePressEvent(event);
}

void Slider::mouseMoveEvent(QMouseEvent *event)
{
  static constexpr double min_ppf = 0.5;
  static constexpr double max_ppf = 70;
  if (m_pan_active) {
    const QPoint d = m_last_mouse_pos - event->pos();
    m_last_mouse_pos = event->pos();
    const double min = pixel_to_frame(frame_to_pixel(m_min) + d.x());
    const double max = pixel_to_frame(frame_to_pixel(m_max) + d.x());
    m_min = min;
    m_max = max;
    update();
  } else if (m_zoom_active) {
    const QPoint d = m_last_mouse_pos - event->pos();
    m_last_mouse_pos = event->pos();
    const double center_frame = pixel_to_frame(m_mouse_down_pos.x() - left_margin);
    const int left = m_mouse_down_pos.x() - left_margin;
    const int right = width() - m_mouse_down_pos.x() - right_margin;
    double ppf = pixel_per_frame() * std::exp(-d.x() / 300.0);
    ppf = std::clamp(ppf, min_ppf, max_ppf);
    m_min = center_frame - left / ppf;
    m_max = center_frame + right  / ppf - 1;
    update();
  } else {
    Q_EMIT value_changed(pixel_to_frame(event->pos().x() - left_margin));
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

const QRectF Slider::content_rect() const
{
  return QRectF(left_margin, top_margin,
                width() - left_margin - right_margin, height() - top_margin - bottom_margin);
}

double Slider::pixel_per_frame() const
{
  return content_rect().width() / static_cast<double>(m_max - m_min + 1);
}

void Slider::draw_lines(QPainter& painter) const
{
  const double pixel_per_frame = this->pixel_per_frame();
  const QRectF content_rect = this->content_rect();
  QPen pen;
  pen.setColor(Qt::black);
  pen.setCosmetic(true);
  painter.setPen(pen);
  for (int frame = m_min; frame <= m_max; ++frame) {
    QFont font;
    QFontMetrics fm(font);
    if (pixel_per_frame < 10 && (frame % 2 != 0)) {
      continue;
    } else if (pixel_per_frame < 2 && frame % 10 != 0) {
      continue;
    }
    pen.setWidthF(frame % 10 == 0 ? 2.0 : 1.0);
    painter.setPen(pen);
    const double x = frame_to_pixel(frame);
    const int line_start = frame % 2 == 0 ? 0 : content_rect.height() / 2.0;
    const int line_end = content_rect.height() - (frame %  10 == 0 ? fm.height() : 0);
    painter.drawLine(x, line_start, x, line_end);
    if (frame % 10 == 0) {
      const QString text = QString("%1").arg(frame);
      painter.drawText(QPointF(x - fm.horizontalAdvance(text)/2.0, content_rect.height()), text);
    }
  }
}

void Slider::draw_current(QPainter& painter) const
{
  const double height = this->content_rect().height();
  const double pixel_per_frame = this->pixel_per_frame();
  const QRectF current_rect(-pixel_per_frame/2.0, height/2.0, pixel_per_frame, height);
  painter.fillRect(current_rect, QColor(255, 128, 0, 60));
  QPen pen;
  pen.setColor(QColor(255, 128, 0, 120));
  pen.setWidthF(4.0);
  painter.setPen(pen);
  painter.drawRect(current_rect);
}


}  // namespace omm
