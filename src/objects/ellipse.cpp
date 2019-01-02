#include "objects/ellipse.h"
#include <QObject>
#include "properties/floatproperty.h"
#include "objects/path.h"

namespace omm
{

class Style;

Ellipse::Ellipse(Scene* scene) : Object(scene)
{
  add_property<FloatProperty>("rx", 100.0).set_label(QObject::tr("rx").toStdString())
                                          .set_category(QObject::tr("ellipse").toStdString());
  add_property<FloatProperty>("ry", 100.0).set_label(QObject::tr("ry").toStdString())
                                          .set_category(QObject::tr("ellipse").toStdString());
}

void Ellipse::render(AbstractRenderer& renderer, const Style& style) const
{
  const size_t n = 10;
  const auto rx = property("rx").value<double>();
  const auto ry = property("ry").value<double>();

  std::vector<Point> points;
  points.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    const double x = cos(i * 2.0/n * M_PI) * rx;
    const double y = sin(i * 2.0/n * M_PI) * ry;
    points.push_back(Point(arma::vec2 { x, y }));
  }
  renderer.draw_spline(points, style, true);
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
  return TYPE;
}

std::unique_ptr<Object> Ellipse::clone() const
{
  return std::make_unique<Ellipse>(*this);
}

Object::Flag Ellipse::flags() const
{
  return Flag::Convertable;
}

std::unique_ptr<Object> Ellipse::convert() const
{
  auto converted = std::make_unique<Path>(scene());
  converted->property(NAME_PROPERTY_KEY).set(name());
  return converted;
}


}  // namespace omm
