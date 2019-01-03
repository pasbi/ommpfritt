#include "objects/proceduralpath.h"
#include <QObject>
#include <pybind11/stl.h>
#include "properties/integerproperty.h"
#include "objects/path.h"
#include "python/pythonengine.h"
#include "scene/scene.h"
#include "python/pointwrapper.h"
#include "python/scenewrapper.h"

namespace omm
{

class Style;

ProceduralPath::ProceduralPath(Scene* scene) : AbstractProceduralPath(scene)
{
  add_property<StringProperty>(CODE_PROPERTY_KEY, "")
    .set_is_multi_line(true)
    .set_label(QObject::tr("code").toStdString())
    .set_category(QObject::tr("ProceduralPath").toStdString());
  add_property<IntegerProperty>(COUNT_PROPERTY_KEY, 10)
    .set_label(QObject::tr("count").toStdString())
    .set_category(QObject::tr("ProceduralPath").toStdString());
}

BoundingBox ProceduralPath::bounding_box()
{
  const auto rx = property("rx").value<double>();
  const auto ry = property("ry").value<double>();
  return BoundingBox({
    arma::vec2 { -rx, -ry },
    arma::vec2 { rx, ry }
  });
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
  const auto point_wrappers = ::transform<PointWrapper>(points, [](auto& point) {
    return PointWrapper(&point);
  });

  if (points.size() > 0) {
    const auto locals = pybind11::dict( "points"_a=point_wrappers,
                                        "scene"_a=SceneWrapper(scene()) );
    scene()->python_engine.run(code, locals);
  }

  return points;
}

bool ProceduralPath::is_closed() const
{
  return true;
}


}  // namespace omm
