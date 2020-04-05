#include "widgets/splinewidget.h"
#include "logging.h"
#include "splinetype.h"
#include <QMouseEvent>
#include <QPainter>

namespace
{
using namespace omm;

static constexpr double closeness_threshold_px = 4;

using Side = SplineType::Knot::Side;
template<typename Iterator>
QPointF knot_pos(Iterator&& it, Side side)
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

  return { (other->first + 2.0 * it->first) / 3.0, it->second.get_value(side) };
}

}  // namespace

namespace omm
{

SplineWidget::SplineWidget(QWidget* parent)
  : QWidget(parent)
{
}

void SplineWidget::set_model(SplineType* spline)
{
  m_model = spline;
  update();
}

void SplineWidget::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  painter.save();
  painter.setTransform(transform());
  if (m_model == nullptr) {
    painter.fillRect(rect(), Qt::gray);
  } else {
    painter.fillRect(rect(), Qt::white);

    painter.save();
    draw_spline(painter);
    painter.restore();
  }
  painter.restore();
}

void SplineWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  update();
}

void SplineWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (m_model != nullptr) {
    if (const auto cp = knot_at(event->pos()); cp.is_valid() && cp.side() == Side::Middle) {
      m_model->knots.erase(cp.iterator());
    } else {
      const QPointF p = transform().inverted().map(QPointF(event->pos()));
      m_model->knots.insert({p.x(), SplineType::Knot(p.y())});
    }
    update();
  }
}

void SplineWidget::mousePressEvent(QMouseEvent* event)
{
  if (m_model != nullptr) {
    if (const auto clicked_knot = knot_at(event->pos()); clicked_knot.is_valid()) {
      m_grabbed_knot = clicked_knot;
    }
  }
}

void SplineWidget::mouseReleaseEvent(QMouseEvent* event)
{
  m_grabbed_knot = m_model->invalid();
}

void SplineWidget::mouseMoveEvent(QMouseEvent* event)
{
  if (m_grabbed_knot.is_valid()) {
    assert(m_model != nullptr);
    const auto p = transform().inverted().map(QPointF(event->pos()));;
    m_grabbed_knot.knot().set_value(m_grabbed_knot.side(), std::clamp(p.y(), 0.0, 1.0));
    if (m_grabbed_knot.side() == Side::Middle) {
      const auto it = m_model->move(m_grabbed_knot.iterator(), std::clamp(p.x(), 0.0, 1.0));
      m_grabbed_knot.iterator() = it;
    }
    update();
  }
}

void SplineWidget::draw_spline(QPainter& painter)
{
  const auto& knots = m_model->knots;
  if (knots.empty()) {
    return;
  }

    painter.save();
  for (auto cp = m_model->begin(); cp.is_valid(); cp.advance()) {
    if (cp.side() != Side::Middle) {
      const QPointF origin = knot_pos(cp.iterator(), Side::Middle);
      const QPointF sat_pos = knot_pos(cp.iterator(), cp.side());
      QPen pen;
      pen.setColor(QColor(0, 0, 0, 128));
      pen.setCosmetic(true);
      pen.setWidthF(0.5);
      painter.setPen(pen);
      painter.drawLine(origin, sat_pos);
    }
  }

  for (auto cp = m_model->begin(); cp.is_valid(); cp.advance()) {
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

  QPainterPath path;
  {
    const auto pos = knot_pos(knots.begin(), Side::Middle);
    path.moveTo(QPointF{0.0, pos.y()});
    path.lineTo(pos);
  }
  for (auto it = std::next(knots.begin()); it != knots.end(); ++it) {
    const QPointF& ctrl_pt_1 = knot_pos(std::prev(it), Side::Right);
    const QPointF& ctrl_pt_2 = knot_pos(it, Side::Left);
    const QPointF& end_pt = knot_pos(it, Side::Middle);
    path.cubicTo(ctrl_pt_1, ctrl_pt_2, end_pt);
  }
  {
    const auto pos = knot_pos(std::prev(knots.end()), Side::Middle);
    path.lineTo(QPointF{1.0, pos.y()});
  }
  QPen pen;
  pen.setCosmetic(true);
  pen.setWidth(2.0);
  pen.setColor(Qt::red);
  painter.setPen(pen);
  painter.drawPath(path);
  painter.restore();
}

QTransform SplineWidget::transform() const
{
  QTransform t;
  t.scale(width(), height());
  return t;
}

SplineType::ControlPoint SplineWidget::knot_at(const QPoint& pos) const
{
  assert(m_model != nullptr);

  const auto is_close = [this, pos](const auto& cp) {
    const QPointF d = this->transform().map(knot_pos(cp.iterator(), cp.side())) - pos;
    return QPointF::dotProduct(d, d) < closeness_threshold_px * closeness_threshold_px;
    return false;
  };

  for (SplineType::ControlPoint cp = m_model->begin(); cp.is_valid(); cp.advance()) {
    if (is_close(cp)) {
      return cp;
    }
  }
  return m_model->invalid();
}

}  // namespace omm
