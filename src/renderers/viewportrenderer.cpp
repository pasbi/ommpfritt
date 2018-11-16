#include "renderers/viewportrenderer.h"
#include "renderers/style.h"

#include <glog/logging.h>
#include <QTransform>
#include <QPainter>

namespace
{

auto to_transformation(const omm::ObjectTransformation& transformation)
{
  const auto& m = transformation.matrix();
  return QTransform( m.at(0, 0), m.at(1, 0), m.at(2, 0),
                     m.at(0, 1), m.at(1, 1), m.at(2, 1),
                     m.at(0, 2), m.at(1, 2), m.at(2, 2) );
}

auto to_point(const arma::vec2& point)
{
  return QPointF(point[0], point[1]);
}

}  // namespace

namespace omm
{

ViewportRenderer::ViewportRenderer(QPainter& painter, const BoundingBox& bounding_box)
  : AbstractRenderer(bounding_box)
  , m_painter(painter)
{
  painter.fillRect( bounding_box.top() + 10, bounding_box.left() + 10,
                    bounding_box.width() - 20, bounding_box.height() - 20,
                    Qt::white );
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
  path.moveTo(to_point(points.front().position));

  for (size_t i = 1; i < points.size(); ++i)
  {
    path.cubicTo( to_point(points.at(i-1).right_position()),
                  to_point(points.at(i).left_position()),
                  to_point(points.at(i).position)  );
  }

  m_painter.strokePath(path, QPen());
  m_painter.fillRect(QRect(0, 0, 10, 10), Qt::red);
}

}  // namespace omm
