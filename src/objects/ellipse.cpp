#include "objects/ellipse.h"

#include <QObject>

#include "properties/floatproperty.h"

namespace omm
{

class Style;

Ellipse::Ellipse()
{
  add_property( "rx", std::make_unique<FloatProperty>(100.0) )
    .set_label(QObject::tr("rx").toStdString())
    .set_category(QObject::tr("ellipse").toStdString());
  add_property( "ry", std::make_unique<FloatProperty>(100.0) )
    .set_label(QObject::tr("ry").toStdString())
    .set_category(QObject::tr("ellipse").toStdString());
}

void Ellipse::render(AbstractRenderer& renderer, const Style& style) const
{
  const size_t n = 10;
  const auto rx = property("rx").value<double>();
  const auto ry = property("ry").value<double>();

  std::vector<Point> points;
  points.reserve(n+1);
  for (size_t i = 0; i <= n; ++i) {
    const double x = cos(i * 2.0/n * M_PI) * rx;
    const double y = sin(i * 2.0/n * M_PI) * ry;
    points.push_back(Point(arma::vec2 { x, y }));
  }
  renderer.draw_spline(points, style);
}

BoundingBox Ellipse::bounding_box() const
{
  const auto rx = property("rx").value<double>();
  const auto ry = property("ry").value<double>();
  return BoundingBox({
    arma::vec2 { -rx, -ry },
    arma::vec2 { rx, ry }
  });
}

std::string Ellipse::type() const
{
  return "Ellipse";
}

}  // namespace omm
