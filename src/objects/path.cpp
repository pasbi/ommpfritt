#include "objects/path.h"

#include <QObject>

#include "properties/boolproperty.h"

namespace omm
{

class Style;

Path::Path(Scene* scene) : Object(scene)
{
  add_property<BoolProperty>("closed").set_label(QObject::tr("closed").toStdString())
                                      .set_category(QObject::tr("path").toStdString());
}

void Path::render(AbstractRenderer& renderer, const Style& style) const
{
  renderer.draw_spline(m_points, style, property("closed").value<bool>());
}

BoundingBox Path::bounding_box() const
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

}  // namespace omm
