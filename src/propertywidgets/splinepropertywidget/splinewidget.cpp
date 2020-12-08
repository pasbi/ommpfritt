#include "propertywidgets/splinepropertywidget/splinewidget.h"
#include "logging.h"
#include "splinetype.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace
{
using namespace omm;

constexpr double closeness_threshold_px = 4;

using Side = SplineType::Knot::Side;
template<typename Iterator> QPointF knot_pos(Iterator&& it, Side side)
{
  auto&& other = [it, side]() {
    switch (side) {
    case Side::Middle:
      return it;
    case Side::Left:
      return std::prev(it);
    case Side::Right:
      return std::next(it);
    default:
      Q_UNREACHABLE();
      return it;
    }
  }();

  static constexpr double t = 1.0 / 3.0;

  return {(t * other->first + (1.0 - t) * it->first), it->second.get_value(side)};
}

}  // namespace

namespace omm
{
SplineWidget::SplineWidget(QWidget* parent) : QWidget(parent)
{
}

void SplineWidget::set_value(const value_type& spline)
{
  m_spline = spline;
  update();
}

void SplineWidget::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  painter.save();
  painter.fillRect(rect(), Qt::white);
  painter.setTransform(transform());
  draw_spline(painter);
  painter.restore();
}

void SplineWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  update();
}

void SplineWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (const auto cp = knot_at(event->pos()); cp.is_valid() && cp.side() == Side::Middle) {
    m_spline.knots.erase(cp.iterator());
  } else {
    const QPointF p = transform().inverted().map(QPointF(event->pos()));
    m_spline.knots.insert({p.x(), SplineType::Knot(p.y())});
  }
  Q_EMIT value_changed(m_spline);
  update();
}

void SplineWidget::mousePressEvent(QMouseEvent* event)
{
  if (const auto clicked_knot = knot_at(event->pos()); clicked_knot.is_valid()) {
    m_grabbed_knot = clicked_knot;
  }
}

void SplineWidget::mouseReleaseEvent(QMouseEvent*)
{
  m_grabbed_knot = SplineType::ControlPoint();
  Q_EMIT value_changed(m_spline);
}

void SplineWidget::mouseMoveEvent(QMouseEvent* event)
{
  if (m_grabbed_knot.is_valid()) {
    const auto p = transform().inverted().map(QPointF(event->pos()));
    ;
    m_grabbed_knot.knot().set_value(m_grabbed_knot.side(), std::clamp(p.y(), 0.0, 1.0));
    if (m_grabbed_knot.side() == Side::Middle) {
      const auto it = m_spline.move(m_grabbed_knot.iterator(), std::clamp(p.x(), 0.0, 1.0));
      m_grabbed_knot.iterator() = it;
    }
    update();
  }
}

void SplineWidget::set_inconsistent_value()
{
  set_value(SplineType());
}

void SplineWidget::draw_spline(QPainter& painter)
{
  const auto& knots = m_spline.knots;
  if (knots.empty()) {
    return;
  }

  painter.save();
  static const QColor HALF_TRANSPARENT{0, 0, 0, 128};
  static constexpr double SPLINE_PEN_WIDTH = 0.5;
  for (auto cp = m_spline.begin(); cp.is_valid(); cp.advance()) {
    if (cp.side() != Side::Middle) {
      const QPointF origin = knot_pos(cp.iterator(), Side::Middle);
      const QPointF sat_pos = knot_pos(cp.iterator(), cp.side());
      QPen pen;
      pen.setColor(HALF_TRANSPARENT);
      pen.setCosmetic(true);
      pen.setWidthF(SPLINE_PEN_WIDTH);
      painter.setPen(pen);
      painter.drawLine(origin, sat_pos);
    }
  }

  for (auto cp = m_spline.begin(); cp.is_valid(); cp.advance()) {
    const QPointF sat_pos = knot_pos(cp.iterator(), cp.side());
    QPainterPath path;
    const auto r = cp.side() == Side::Middle ? 5.0 : 4.0;
    const auto color = cp.side() == Side::Middle ? QColor(0, 128, 255) : QColor(255, 128, 0);
    path.addEllipse(QPointF{0.0, 0.0}, r, r);

    const auto gpos = painter.transform().map(sat_pos);
    painter.save();
    painter.resetTransform();
    painter.translate(gpos);
    painter.fillPath(path, color);
    painter.restore();
  }

  {
    QPainterPath path;
    std::size_t n = width() / 2;
    path.moveTo({0.0, m_spline.evaluate(0).value()});
    for (std::size_t i = 1; i <= n; ++i) {
      const double t = static_cast<double>(i) / static_cast<double>(n);
      const double v = m_spline.evaluate(t).value();
      path.lineTo({t, v});
    }
    QPen pen;
    pen.setCosmetic(true);
    pen.setWidth(2.0);
    pen.setColor(Qt::red);
    painter.setPen(pen);
    painter.drawPath(path);
  }
  painter.restore();
}

QTransform SplineWidget::transform() const
{
  QTransform t;
  t.scale(width(), height());
  t.translate(0.0, 1.0);
  t.scale(1.0, -1.0);
  return t;
}

SplineType::ControlPoint SplineWidget::knot_at(const QPoint& pos)
{
  const auto is_close = [this, pos](const auto& cp) {
    const QPointF d = this->transform().map(knot_pos(cp.iterator(), cp.side())) - pos;
    return QPointF::dotProduct(d, d) < closeness_threshold_px * closeness_threshold_px;
  };

  for (SplineType::ControlPoint cp = m_spline.begin(); cp.is_valid(); cp.advance()) {
    if (is_close(cp)) {
      return cp;
    }
  }
  return SplineType::ControlPoint{};
}

}  // namespace omm
