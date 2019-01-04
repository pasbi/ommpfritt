#include "objects/path.h"

#include <QObject>

#include "properties/boolproperty.h"

namespace omm
{

class Style;

Path::Path(Scene* scene) : Object(scene)
{
  add_property<BoolProperty>(IS_CLOSED_PROPERTY_KEY)
    .set_label(QObject::tr("closed").toStdString())
    .set_category(QObject::tr("path").toStdString());
}

void Path::render(AbstractRenderer& renderer, const Style& style)
{
  renderer.draw_spline(m_points, style, property("closed").value<bool>());
}

BoundingBox Path::bounding_box()
{
  return BoundingBox(); // TODO
}

std::string Path::type() const
{
  return TYPE;
}

std::unique_ptr<Object> Path::clone() const
{
  return std::make_unique<Path>(*this);
}

void Path::set_points(const std::vector<Point>& points)
{
  m_points = points;
}

std::vector<Point*> Path::points()
{
  return ::transform<Point*>(m_points, [](Point& p) { return &p; });
}

}  // namespace omm
