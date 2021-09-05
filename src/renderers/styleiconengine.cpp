#include "renderers/styleiconengine.h"
#include "preferences/uicolors.h"
#include "renderers/painter.h"
#include <QPainter>
#include <memory>

namespace
{
double adjust_pen_width(double width, const QSize& size)
{
  const double max_width = std::min(size.width(), size.height());

  // f_a(x) := ax/(x+a)
  // f(0) = 0, f(inf) = a,  f'(0) = 1
  // i.e., for small widths (aka x), the function behaves like identity.
  // for large widths, it converges agains max_width (aka a).
  return max_width * width / (max_width + width);
}

void draw_style(QPainter& painter, const QRect& rect, const omm::Style& style)
{
  painter.save();
  painter.setBrush(omm::Painter::make_simple_brush(style));
  auto pen = omm::Painter::make_simple_pen(style);
  pen.setWidthF(adjust_pen_width(pen.width(), rect.size()));
  painter.setPen(pen);
  const auto r = 0.8 * std::min(rect.size().width(), rect.size().height());
  const auto ir = static_cast<int>(r);
  painter.drawEllipse(rect.bottomRight(), ir, ir);

  painter.restore();
}

}  // namespace

namespace omm
{
StyleIconEngine::StyleIconEngine(const Style* style) : m_style(style)
{
}

QIconEngine* StyleIconEngine::clone() const
{
  return std::make_unique<StyleIconEngine>(*this).release();
}

void StyleIconEngine::paint(QPainter* painter, const QRect& rect, QIcon::Mode, QIcon::State)
{
  painter->save();
  painter->setClipRect(rect);
  UiColors::draw_background(*painter, rect);
  if (m_style != nullptr) {
    draw_style(*painter, rect, *m_style);
  } else {
    painter->save();
    QPen pen(Qt::black);
    pen.setWidth(3);
    pen.setCosmetic(true);
    painter->setPen(pen);
    const QPointF margin = 0.2 * QPointF(rect.width(), rect.height());
    QRectF inset = rect;
    inset.adjust(margin.x(), margin.y(), -margin.x(), -margin.y());
    painter->drawLine(inset.topLeft(), inset.bottomRight());
    painter->drawLine(inset.topRight(), inset.bottomLeft());
    painter->restore();
  }
  painter->restore();
}

}  // namespace omm
