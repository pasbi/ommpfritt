#include "managers/timeline/slider.h"
#include "logging.h"
#include <QMouseEvent>
#include <QPainter>
#include <cmath>

namespace
{


}  // namespace

namespace omm
{

Slider::Slider(QWidget *parent) : QWidget(parent), m_min(1), m_max(100), m_value(1)
{
}

void Slider::set_value(int frame)
{
  frame = std::clamp(frame, m_min, m_max);
  if (m_value != frame) {
    m_value = frame;
    update();
    Q_EMIT value_changed(frame);
  }
}

void Slider::set_min(int frame)
{
  if (frame != m_min) {
    m_min = frame;
    if (m_value < m_min) {
      set_value(m_min);
    }
    if (m_max < m_min) {
      set_max(m_min+1);
    }
    update();
    Q_EMIT min_changed(frame);
  }
}

void Slider::set_max(int frame)
{
  if (frame != m_max) {
    m_max = frame;
    if (m_value > m_max) {
      set_value(m_max);
    }
    if (m_max < m_min) {
      set_min(m_max-1);
    }
    update();
    Q_EMIT max_changed(frame);
  }
}

void Slider::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  painter.fillRect(rect(), Qt::white);

  painter.save();
  painter.translate(left_margin, bottom_margin);

  painter.save();
  draw_lines(painter);
  painter.restore();

  painter.save();
  painter.translate(frame_to_pixel(m_value), 0);
  draw_current(painter);
  painter.restore();

  painter.restore();

  QWidget::paintEvent(event);
}

void Slider::mousePressEvent(QMouseEvent *event)
{
  const int x = event->pos().x() - left_margin;
  m_handle_grabbed = std::abs(frame_to_pixel(m_value) - x) < pixel_per_frame() / 2.0;
  QWidget::mousePressEvent(event);
}

void Slider::mouseMoveEvent(QMouseEvent *event)
{
  if (m_handle_grabbed) {
    set_value(pixel_to_frame(event->pos().x() - left_margin));
  }
  QWidget::mouseMoveEvent(event);
}

void Slider::mouseReleaseEvent(QMouseEvent *event)
{
  m_handle_grabbed = false;
  QWidget::mouseReleaseEvent(event);
}

double Slider::frame_to_pixel(int frame) const
{
  const double t = static_cast<double>(frame - m_min) / static_cast<double>(m_max - m_min + 1);
  return t * content_rect().width();
}

int Slider::pixel_to_frame(int pixel) const
{
  double frame = static_cast<double>(pixel) / content_rect().width();
  frame *= (m_max - m_min + 1);
  frame += m_min;
  return std::round(frame);
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
