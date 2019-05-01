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

namespace
{

constexpr auto default_script = R"(import math
import numpy as np

for i, p in enumerate(points):
  r = 50 if i % 2 else 200
  theta = i/len(points)*math.pi*2
  pos = np.array([math.cos(theta), math.sin(theta)])
  tangent = np.array([pos[1], -pos[0]])
  p.set_position(r*pos)
  r /= 10
  p.set_left_tangent(r*tangent)
  p.set_right_tangent(-r*tangent)
)";

}  // namespace

namespace omm
{

class Style;

ProceduralPath::ProceduralPath(Scene* scene) : AbstractProceduralPath(scene)
{
  static const auto category = QObject::tr("ProceduralPath").toStdString();
  add_property<StringProperty>(CODE_PROPERTY_KEY, default_script)
    .set_mode(StringProperty::Mode::Code)
    .set_label(QObject::tr("code").toStdString()).set_category(category);
  add_property<IntegerProperty>(COUNT_PROPERTY_KEY, 10)
    .set_range(0, std::numeric_limits<int>::max())
    .set_label(QObject::tr("count").toStdString()).set_category(category);
  add_property<BoolProperty>(IS_CLOSED_PROPERTY_KEY)
    .set_label(QObject::tr("closed").toStdString()).set_category(category);
}


std::unique_ptr<Object> ProceduralPath::clone() const
{
  return std::make_unique<ProceduralPath>(*this);
}

std::string ProceduralPath::type() const { return TYPE; }
std::vector<Point> ProceduralPath::points() const { return m_points; }

void ProceduralPath::update()
{
  assert(scene() != nullptr);
  using namespace pybind11::literals;
  const auto count = property(COUNT_PROPERTY_KEY)->value<int>();
  const auto code = property(CODE_PROPERTY_KEY)->value<std::string>();

  m_points = std::vector<Point>(static_cast<std::size_t>(std::max(0, count)));
  std::vector<PointWrapper> point_wrappers;
  point_wrappers.reserve(m_points.size());
  for (Point& point : m_points) {
    point_wrappers.emplace_back(point);
  }

  if (m_points.size() > 0) {
    const auto locals = pybind11::dict( "points"_a=point_wrappers,
                                        "this"_a=ObjectWrapper::make(*this),
                                        "scene"_a=SceneWrapper(*scene()) );
    scene()->python_engine.exec(code, locals, this);
  }
}

bool ProceduralPath::is_closed() const
{
  return property(IS_CLOSED_PROPERTY_KEY)->value<bool>();
}

AbstractPropertyOwner::Flag ProceduralPath::flags() const
{
  return Object::flags() | Flag::IsPathLike | Flag::HasScript | Flag::Convertable;
}

}  // namespace omm
