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
  QPen pen;
  pen.setWidth(style.property<omm::FloatProperty>(omm::Style::PEN_WIDTH_KEY).value());
  pen.setColor(to_qcolor(style.property<omm::ColorProperty>(omm::Style::PEN_COLOR_KEY).value()));
  return pen;
}

auto make_brush(const omm::Style& style)
{
  QBrush brush(Qt::SolidPattern);
  const auto color = style.property<omm::ColorProperty>(omm::Style::BRUSH_COLOR_KEY).value();
  brush.setColor(to_qcolor(color));
  return brush;
}

}  // namespace

namespace omm
{

ViewportRenderer::ViewportRenderer(QPainter& painter, const BoundingBox& bounding_box)
  : AbstractRenderer(bounding_box)
  , m_painter(painter)
{
}

void ViewportRenderer::push_transformation(const ObjectTransformation& transformation)
{
  AbstractRenderer::push_transformation(transformation);
  m_painter.setTransform(to_transformation(current_transformation()), false);
}

void ViewportRenderer::pop_transformation()
{
  AbstractRenderer::pop_transformation();
  m_painter.setTransform(to_transformation(current_transformation()), false);
}

void ViewportRenderer::draw_spline(const std::vector<Point>& points, const Style& style)
{
  QPainterPath path;
  path.moveTo(to_qpoint(points.front().position));

  for (size_t i = 1; i < points.size(); ++i)
  {
    path.cubicTo( to_qpoint(points.at(i-1).right_position()),
                  to_qpoint(points.at(i).left_position()),
                  to_qpoint(points.at(i).position)  );
  }

  if (style.property<BoolProperty>(Style::BRUSH_IS_ACTIVE_KEY).value()) {
    m_painter.fillPath(path, make_brush(style));
  }
  if (style.property<BoolProperty>(Style::PEN_IS_ACTIVE_KEY).value()) {
    m_painter.strokePath(path, make_pen(style));
  }
}

}  // namespace omm
