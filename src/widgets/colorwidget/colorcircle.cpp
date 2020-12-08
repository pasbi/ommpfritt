#include "widgets/colorwidget/colorcircle.h"
#include "logging.h"
#include "preferences/uicolors.h"
#include <QCloseEvent>
#include <QPainter>
#include <cmath>
#include <complex>

namespace
{
std::array<double, 2>
barycentric(const QPointF& p, const QPointF& a, const QPointF& b, const QPointF& c)
{
  const QPointF v0 = b - a;
  const QPointF v1 = c - a;
  const QPointF v2 = p - a;
  double d00 = QPointF::dotProduct(v0, v0);
  double d01 = QPointF::dotProduct(v0, v1);
  double d11 = QPointF::dotProduct(v1, v1);
  double d20 = QPointF::dotProduct(v2, v0);
  double d21 = QPointF::dotProduct(v2, v1);
  double denom = d00 * d11 - d01 * d01;
  return {
      (d11 * d20 - d01 * d21) / denom,
      (d00 * d21 - d01 * d20) / denom,
  };
}

std::array<double, 2> polar(const QPointF& p)
{
  return {std::sqrt(std::pow(p.x(), 2.0) + std::pow(p.y(), 2.0)), std::atan2(p.y(), p.x())};
}

constexpr double circle_width = 0.2;

QPainterPath make_circle()
{
  QPainterPath circle;
  double d = circle_width / 2.0;
  static constexpr QRectF rect(QPointF(-0.5, -0.5), QPointF(0.5, 0.5));
  circle.addEllipse(rect);
  circle.addEllipse(rect.adjusted(d, d, -d, -d));
  return circle;
}

std::array<QPointF, 3> make_triangle(double hue)
{
  const double alpha = -2 * M_PI * hue;
  const double r = 0.5 - circle_width / 2.0;
  const double a = 0.0 / 3.0 * M_PI + alpha;
  const double b = 2.0 / 3.0 * M_PI + alpha;
  const double c = 4.0 / 3.0 * M_PI + alpha;

  const QPointF pa = QPointF(std::cos(a), std::sin(a)) * r;
  const QPointF pb = QPointF(std::cos(b), std::sin(b)) * r;
  const QPointF pc = QPointF(std::cos(c), std::sin(c)) * r;

  return {pa, pb, pc};
}

}  // namespace

namespace omm
{
ColorCircle::ColorCircle(QWidget* parent) : ColorPicker(parent), m_circle(make_circle())
{
}

void ColorCircle::set_color(const Color& color)
{
  ColorPicker::set_color(color);
  update();
}

void ColorCircle::mouseMoveEvent(QMouseEvent* e)
{
  set_color(e->pos());
}

void ColorCircle::mouseReleaseEvent(QMouseEvent*)
{
  m_drag_circle = false;
  m_drag_triangle = false;
}

QPointF ColorCircle::to_relative(const QPoint& pos) const
{
  const QSizeF size = this->size();
  double scale = std::min(size.width(), size.height());
  return (pos - QPointF(size.width() / 2.0, size.height() / 2.0)) / scale * 2.0;
}

void ColorCircle::set_color(const QPoint& p)
{
  const QPointF pos = to_relative(p);
  const auto [hue, sat, val, alpha] = color().components(Color::Model::HSVA);

  if (m_drag_circle) {
    const auto [r, arg] = polar(pos);
    Color color = this->color();
    color.set(Color::Role::Hue, -fmod(arg / (2.0 * M_PI) + 1.0, 1.0));
    set_color(color);
  } else {
    const auto [a, b, c] = make_triangle(hue);
    auto [u, v] = barycentric(pos / 2.0, a, b, c);
    u = std::clamp(u, 0.0, 1.0);
    v = std::clamp(v, 0.0, 1.0);
    const double w = std::clamp(1.0 - u - v, 0.0, 1.0);
    using Model = Color::Model;
    auto components = Color(Model::HSVA, {hue, 1.0, 1.0, alpha}).components(Model::RGBA);

    for (std::size_t i = 0; i < 3; ++i) {
      components.at(i) = w * components.at(i) + u * 0.0 + v * 1.0;
    }
    Color color(Model::RGBA, components);
    color.set(Color::Role::Hue, hue);
    set_color(color);
  }
}

void ColorCircle::mousePressEvent(QMouseEvent* e)
{
  const QPointF pos = to_relative(e->pos());
  const auto [r, arg] = polar(pos);

  m_drag_circle = false;
  if (r > (1.0 - circle_width) && r < 1.0) {
    m_drag_circle = true;
  }

  set_color(e->pos());
}

void ColorCircle::paintEvent(QPaintEvent* e)
{
  Q_UNUSED(e)
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QTransform transform;
  transform.translate(width() / 2., height() / 2.0);
  double scale = std::min(width(), height());
  transform.scale(scale, scale);

  auto [hue, sat, val, alpha] = color().components(Color::Model::HSVA);
  auto [pf_c, pf_b, pf_w] = make_triangle(hue);
  const QPointF p_c = transform.map(pf_c);
  const QPointF p_b = transform.map(pf_b);
  const QPointF p_w = transform.map(pf_w);

  QConicalGradient circle_gradient;
  for (double d : {0.0, 1.0 / 6.0, 2.0 / 6.0, 0.5, 3.0 / 6.0, 4.0 / 6.0, 1.0}) {
    circle_gradient.setColorAt(d, Color(Color::Model::HSVA, {d, 1.0, 1.0, 1.0}).to_qcolor());
  }
  circle_gradient.setCenter(transform.map(QPoint()));
  painter.fillPath(transform.map(m_circle), circle_gradient);

  QLinearGradient lg_hue;
  lg_hue.setCoordinateMode(QGradient::LogicalMode);
  lg_hue.setColorAt(0.0, Color(Color::Model::HSVA, {hue, 1.0, 1.0, alpha}).to_qcolor());
  lg_hue.setColorAt(1.0, Color(Color::Model::HSVA, {hue, 0.0, 1.0, alpha}).to_qcolor());
  lg_hue.setStart(p_c);
  lg_hue.setFinalStop((p_w + p_b) / 2.0);

  QLinearGradient lg_white;
  lg_white.setCoordinateMode(QGradient::LogicalMode);
  lg_white.setColorAt(0.0, Color(Color::Model::HSVA, {hue, 0.0, 1.0, alpha}).to_qcolor());
  lg_white.setColorAt(1.0, Color(Color::Model::HSVA, {hue, 0.0, 0.0, alpha}).to_qcolor());
  lg_white.setStart(p_c);
  lg_white.setFinalStop((p_w + p_b) / 2.0);

  QLinearGradient lg_val;
  lg_val.setCoordinateMode(QGradient::LogicalMode);
  lg_val.setColorAt(0.0, Color(Color::Model::HSVA, {hue, 0.0, 0.0, alpha}).to_qcolor());
  lg_val.setColorAt(1.0, Color(Color::Model::HSVA, {hue, 0.0, 1.0, alpha}).to_qcolor());
  lg_val.setStart(p_b);
  lg_val.setFinalStop(p_w);

  const QPolygonF triangle({p_c, p_b, p_w});
  QPainterPath triangle_path;
  triangle_path.addPolygon(triangle);

  {
    painter.save();
    const QRect rect = this->rect();
    QImage image(rect.size(), QImage::Format_ARGB32_Premultiplied);
    QPainter tpainter(&image);
    tpainter.setRenderHint(QPainter::Antialiasing);
    UiColors::draw_background(tpainter, rect);
    QPainterPath all_but_triangle;
    all_but_triangle.addRect(rect);
    all_but_triangle.addPath(triangle_path);
    tpainter.setCompositionMode(QPainter::CompositionMode_SourceOut);
    tpainter.fillPath(all_but_triangle, QColor(0, 0, 0, 0));
    painter.drawImage(rect, image, rect);
    painter.restore();
  }
  {
    painter.fillPath(triangle_path, lg_val);
    painter.save();
    painter.setCompositionMode(QPainter::CompositionMode_Plus);
    painter.fillPath(triangle_path, lg_white);
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.fillPath(triangle_path, lg_hue);
    painter.restore();
  }

  QPainterPath dot;
  static constexpr double RADIUS = 5;
  dot.addEllipse(QRectF(QPointF(-RADIUS, -RADIUS), QPointF(RADIUS, RADIUS)));
  {
    const QPointF c = val * (sat * (p_c - p_w) + p_w - p_b) + p_b;
    painter.translate(c.x(), c.y());
  }

  QPen pen;
  pen.setColor(ui_color(*this, QPalette::Text));
  pen.setWidth(2.0);
  painter.setPen(pen);
  painter.fillPath(dot, ui_color(*this, QPalette::Base));
  painter.drawPath(dot);
}

}  // namespace omm
