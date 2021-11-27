#include "objects/boolean.h"
#include "objects/enhancedpathvector.h"
#include "properties/optionproperty.h"
#include <2geom/2geom.h>
#include <2geom/intersection-graph.h>
#include <2geom/pathvector.h>
#include <2geom/utils.h>

namespace
{
using F = std::function<Geom::PathVector(Geom::PathIntersectionGraph&)>;
const std::vector<std::pair<QString, F>> dispatcher = {
    {QObject::tr("Union"), [](auto& pig) { return pig.getUnion(); }},
    {QObject::tr("Intersection"), [](auto& pig) { return pig.getIntersection(); }},
    {QObject::tr("Exclusive Or"), [](auto& pig) { return pig.getXOR(); }},
    {QObject::tr("Difference"), [](auto& pig) { return pig.getAminusB(); }},
    {QObject::tr("Inverse Difference"), [](auto& pig) { return pig.getBminusA(); }},
};

}  // namespace

namespace omm
{
Boolean::Boolean(Scene* scene) : Object(scene)
{
  create_property<OptionProperty>(MODE_PROPERTY_KEY)
      .set_options(::transform<QString>(dispatcher, [](auto&& p) { return p.first; }))
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

EnhancedPathVector Boolean::paths() const
{
  const auto children = tree_children();
  if (is_active() && children.size() == 2) {
    static constexpr auto get_path_vector = [](const Object& object) {
      const auto t = object.transformation();
      return t.apply(object.geom_paths().path_vector());
    };
    Geom::PathIntersectionGraph pig{get_path_vector(*children[0]), get_path_vector(*children[1])};
    if (pig.valid()) {
      const auto i = property(MODE_PROPERTY_KEY)->value<std::size_t>();
      return dispatcher[i].second(pig);
    } else {
      return {};
    }
  } else {
    return {};
  }
}

}  // namespace omm
