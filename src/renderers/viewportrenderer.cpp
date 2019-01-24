#include "renderers/viewportrenderer.h"

#include <glog/logging.h>
#include <QTransform>
#include <QPainter>

#include "renderers/style.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"
#include "properties/boolproperty.h"

namespace
{

auto to_transformation(const omm::ObjectTransformation& transformation)
{
  const auto& m = transformation.to_mat();
  return QTransform( m.at(0, 0), m.at(1, 0), m.at(2, 0),
                     m.at(0, 1), m.at(1, 1), m.at(2, 1),
                     m.at(0, 2), m.at(1, 2), m.at(2, 2) );
}

auto to_qpoint(const arma::vec2& point)
{
  return QPointF(point[0], point[1]);
}

auto to_qcolor(omm::Color color)
{
  color = color.clamped() * 255.0;
  return QColor(color.red(), color.green(), color.blue(), color.alpha());
}

auto make_pen(const omm::Style& style)
{
  if (style.property(omm::Style::PEN_IS_ACTIVE_KEY).value<bool>()) {
    QPen pen;
    pen.setWidth(style.property(omm::Style::PEN_WIDTH_KEY).value<double>());
    pen.setColor(to_qcolor(style.property(omm::Style::PEN_COLOR_KEY).value<omm::Color>()));
    return pen;
  } else {
    return QPen(Qt::NoPen);
  }
}

auto make_brush(const omm::Style& style)
{
  if (style.property(omm::Style::BRUSH_IS_ACTIVE_KEY).value<bool>()) {
    QBrush brush(Qt::SolidPattern);
    const auto color = style.property(omm::Style::BRUSH_COLOR_KEY).value<omm::Color>();
    brush.setColor(to_qcolor(color));
    return brush;
  } else {
    return QBrush(Qt::NoBrush);
  }
}

}  // namespace

namespace omm
{

ViewportRenderer::ViewportRenderer(Scene& scene) : AbstractRenderer(scene) {}

void ViewportRenderer::push_transformation(const ObjectTransformation& transformation)
{
  AbstractRenderer::push_transformation(transformation);
  m_painter->setTransform(to_transformation(current_transformation()), false);
}

void ViewportRenderer::pop_transformation()
{
  AbstractRenderer::pop_transformation();
  m_painter->setTransform(to_transformation(current_transformation()), false);
}

void
ViewportRenderer::draw_spline(const std::vector<Point>& points, const Style& style, bool closed)
{
  QPainterPath path;
  if (points.size() > 1) {
    path.moveTo(to_qpoint(points.front().position));

    for (size_t i = 1; i < points.size(); ++i)
    {
      path.cubicTo( to_qpoint(points.at(i-1).right_position()),
                    to_qpoint(points.at(i).left_position()),
                    to_qpoint(points.at(i).position)  );
    }

    if (closed && points.size() > 2) {
      path.cubicTo(to_qpoint(points.back().right_position()),
                   to_qpoint(points.front().left_position()),
                   to_qpoint(points.front().position) );
    }

    if (style.property(Style::BRUSH_IS_ACTIVE_KEY).value<bool>()) {
      m_painter->fillPath(path, make_brush(style));
    }
    if (style.property(Style::PEN_IS_ACTIVE_KEY).value<bool>()) {
      m_painter->strokePath(path, make_pen(style));
    }
  }
}

void
ViewportRenderer::draw_rectangle(const arma::vec2& pos, const double radius, const Style& style)
{
  // TODO I guess using QPainter::drawRect is faster.
  // However, QPainter::drawRect interface is strange, so using it is not trivial, but it shouldn't
  //  be too hard, either.
  const auto tl = Point( pos + arma::vec2 { -radius, -radius } );
  const auto tr = Point( pos + arma::vec2 {  radius, -radius } );
  const auto bl = Point( pos + arma::vec2 { -radius,  radius } );
  const auto br = Point( pos + arma::vec2 {  radius,  radius } );
  draw_spline({ tl, tr, br, bl }, style, true);
}

void ViewportRenderer::draw_circle(const arma::vec2& pos, const double radius, const Style& style)
{
  m_painter->save();
  m_painter->setPen(make_pen(style));
  m_painter->setBrush(make_brush(style));
  m_painter->drawEllipse(to_qpoint(pos), radius, radius);
  m_painter->restore();
}


void ViewportRenderer::set_painter(QPainter& painter) { m_painter = &painter; }
void ViewportRenderer::clear_painter() { m_painter = nullptr; }

}  // namespace omm
