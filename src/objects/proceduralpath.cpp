#include "objects/proceduralpath.h"
#include <QObject>
#include <pybind11/stl.h>
#include "properties/integerproperty.h"
#include "properties/boolproperty.h"
#include "objects/path.h"
#include "python/pythonengine.h"
#include "scene/scene.h"
#include "python/pointwrapper.h"
#include "python/objectwrapper.h"
#include "python/scenewrapper.h"

namespace omm
{

class Style;

ProceduralPath::ProceduralPath(Scene* scene) : AbstractProceduralPath(scene)
{
  add_property<StringProperty>(CODE_PROPERTY_KEY, "")
    .set_mode(StringProperty::Mode::Code)
    .set_label(QObject::tr("code").toStdString())
    .set_category(QObject::tr("ProceduralPath").toStdString());
  add_property<IntegerProperty>(COUNT_PROPERTY_KEY, 10)
    .set_label(QObject::tr("count").toStdString())
    .set_category(QObject::tr("ProceduralPath").toStdString());
  add_property<BoolProperty>(IS_CLOSED_PROPERTY_KEY)
    .set_label(QObject::tr("closed").toStdString())
    .set_category(QObject::tr("ProceduralPath").toStdString());
}

std::string ProceduralPath::type() const
{
  return TYPE;
}

std::unique_ptr<Object> ProceduralPath::clone() const
{
  return std::make_unique<ProceduralPath>(*this);
}

std::vector<Point> ProceduralPath::points()
{
  assert(scene() != nullptr);
  using namespace pybind11::literals;
  const auto count = property(COUNT_PROPERTY_KEY).value<int>();
  const auto code = property(CODE_PROPERTY_KEY).value<std::string>();

  std::vector<Point> points(std::max(0, count));
  std::vector<PointWrapper> point_wrappers;
  point_wrappers.reserve(points.size());
  for (Point& point : points) {
    point_wrappers.emplace_back(point);
  }

  if (points.size() > 0) {
    const auto locals = pybind11::dict( "points"_a=point_wrappers,
                                        "this"_a=ObjectWrapper::make(*this),
                                        "scene"_a=SceneWrapper(*scene()) );
    scene()->python_engine.run(code, locals);
  }

  return points;
}

bool ProceduralPath::is_closed() const
{
  return property(IS_CLOSED_PROPERTY_KEY).value<bool>();
}


}  // namespace omm
