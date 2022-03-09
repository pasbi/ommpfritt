#include "objects/proceduralpath.h"
#include "external/pybind11/stl.h"
#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "path/path.h"
#include "path/pathvector.h"
#include "properties/boolproperty.h"
#include "properties/integerproperty.h"
#include "properties/stringproperty.h"
#include "python/objectwrapper.h"
#include "python/pointwrapper.h"
#include "python/pythonengine.h"
#include "python/scenewrapper.h"
#include "scene/scene.h"
#include "scene/disjointpathpointsetforest.h"
#include <QObject>

namespace
{
constexpr auto default_script = R"(import math
import numpy as np
import omm

n = 20;
points = []
for j in range(2):
    points.append([])
    offset = [j * 300, 0]
    for i in range(n + 1):
        r = 50 if i % 2 else 200
        theta = i/n * math.pi*2
        pos = np.array([math.cos(theta), math.sin(theta)])
        tangent = np.array([pos[1], -pos[0]])
        p = omm.Point()
        p.set_position(r * pos + offset)
        r /= 10
        p.set_left_tangent(r*tangent)
        p.set_right_tangent(-r*tangent)
        points[-1].append(p)
joined_points = [{0, n}, {n+1, 2 * n + 1}]
)";

}  // namespace

namespace omm
{

class Style;

ProceduralPath::ProceduralPath(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("ProceduralPath");
  create_property<StringProperty>(CODE_PROPERTY_KEY, default_script)
      .set_mode(StringProperty::Mode::Code)
      .set_label(QObject::tr("code"))
      .set_category(category);
  ProceduralPath::update();
}

QString ProceduralPath::type() const
{
  return TYPE;
}

void ProceduralPath::update()
{
  assert(scene() != nullptr);
  using namespace pybind11::literals;
  const auto code = property(CODE_PROPERTY_KEY)->value<QString>();

  auto locals = pybind11::dict("this"_a = ObjectWrapper::make(*this),
                               "scene"_a = SceneWrapper(*scene()));

  m_points.clear();
  try {
    PythonEngine::instance().exec(code, locals, this);
    const auto wrappers = locals["points"].cast<std::vector<std::vector<PointWrapper>>>();
    m_joined_points = locals["joined_points"].cast<std::vector<std::set<int>>>();
    for (const auto& ws : wrappers) {
      auto& points = m_points.emplace_back();
      for (const auto& w : ws) {
        points.emplace_back(w.point());
      }
    }

  } catch (const py::error_already_set& e) {
    LERROR << e.what();
  } catch (const py::cast_error& e) {
    LERROR << e.what();
  }

  Object::update();
}

PathVector ProceduralPath::compute_path_vector() const
{
  PathVector pv;
  for (const auto& points : m_points) {
    pv.add_path(std::make_unique<Path>(std::deque(points)));
  }

  try {
    for (const auto& index_set : m_joined_points) {
      pv.joined_points().insert(util::transform<::transparent_set>(index_set, [&pv](const int i) {
        return &pv.point_at_index(i);
      }));
    }
  } catch (const std::runtime_error& e) {
    LERROR << e.what();
  }

  return pv;
}

void ProceduralPath::on_property_value_changed(Property* property)
{
  if (property == this->property(CODE_PROPERTY_KEY) || property == this->property(COUNT_PROPERTY_KEY)) {
    update();
  } else {
    Object::on_property_value_changed(property);
  }
}

Flag ProceduralPath::flags() const
{
  return Object::flags() | Flag::HasScript;
}

}  // namespace omm
