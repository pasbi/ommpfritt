#include "objects/boolean.h"
#include "path/pathvector.h"
#include "properties/optionproperty.h"
#include <2geom/2geom.h>
#include <2geom/intersection-graph.h>
#include <2geom/pathvector.h>
#include <2geom/utils.h>
#include <QApplication>

namespace
{

using F = Geom::PathVector(Geom::PathIntersectionGraph::*)();

class BooleanOperation
{
public:
  constexpr explicit BooleanOperation(std::string_view name, F f)
    : m_name(name)
    , m_f(f)
  {
  }

  Geom::PathVector compute(Geom::PathIntersectionGraph& pig) const
  {
    return std::invoke(m_f, pig);
  }

  QString label() const
  {
    return QApplication::translate("QObject", m_name.data());
  }

private:
  const std::string_view m_name;
  const F m_f;
};

const auto dispatcher = std::array {
    BooleanOperation{"Union", &Geom::PathIntersectionGraph::getUnion},
    BooleanOperation{"Intersection", &Geom::PathIntersectionGraph::getIntersection},
    BooleanOperation{"Exclusive Or", &Geom::PathIntersectionGraph::getXOR},
    BooleanOperation{"Difference", &Geom::PathIntersectionGraph::getAminusB},
    BooleanOperation{"Inverse Difference", &Geom::PathIntersectionGraph::getBminusA},
};

}  // namespace

namespace omm
{
Boolean::Boolean(Scene* scene) : Object(scene)
{
  create_property<OptionProperty>(MODE_PROPERTY_KEY)
      .set_options(::transform<QString, std::vector>(dispatcher, std::mem_fn(&BooleanOperation::label)))
      .set_label(QObject::tr("mode"))
      .set_category(QObject::tr("Boolean"));
  polish();
}

Boolean::Boolean(const Boolean& other) : Object(other)
{
  polish();
}

QString Boolean::type() const
{
  return TYPE;
}

void Boolean::update()
{
  m_draw_children = !is_active();
  Object::update();
}

void Boolean::on_property_value_changed(Property* property)
{
  if (property == this->property(MODE_PROPERTY_KEY)) {
    update();
  }
  Object::on_property_value_changed(property);
}

void Boolean::polish()
{
  listen_to_children_changes();
}

PathVector Boolean::compute_path_vector() const
{
  const auto children = tree_children();
  if (is_active() && children.size() == 2) {
    static constexpr auto get_path_vector = [](const Object& object) {
      const auto t = object.transformation();
      return t.apply(object.path_vector().to_geom());
    };
    Geom::PathIntersectionGraph pig{get_path_vector(*children[0]), get_path_vector(*children[1])};
    if (pig.valid()) {
      const auto i = property(MODE_PROPERTY_KEY)->value<std::size_t>();
      PathVector pv;
      pv.set(dispatcher[i].compute(pig));
      return pv;
    } else {
      return {};
    }
  } else {
    return {};
  }
}

}  // namespace omm
