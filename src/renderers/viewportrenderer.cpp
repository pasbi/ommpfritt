#include "renderers/viewportrenderer.h"
#include "renderers/style.h"

#include <glog/logging.h>
#include <QTransform>
#include <QPainter>

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

auto to_qcolor(const omm::Color& color)
{
  return QColor(255 * color(0), 255 * color(1), 255 * color(2), 255 * color(3));
}

auto make_pen(const omm::Style& style)
{
  QPen pen;
  pen.setWidth(style.pen_width);
  pen.setColor(to_qcolor(style.pen_color));
  return pen;
}

auto make_brush(const omm::Style& style)
{
  QBrush brush;
  brush.setColor(to_qcolor(style.brush_color));
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

  if (style.is_pen_active) {
    m_painter.strokePath(path, make_pen(style));
  }
  if (style.is_brush_active) {
    m_painter.fillPath(path, make_brush(style));
  }
}

}  // namespace omm
