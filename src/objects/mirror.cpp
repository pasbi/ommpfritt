#include "objects/mirror.h"

#include <QObject>
#include "objects/empty.h"
#include "properties/optionsproperty.h"
#include "geometry/vec2.h"

namespace
{


}  // namespace

namespace omm
{

class Style;

Mirror::Mirror(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("Mirror").toStdString();
  add_property<OptionsProperty>(DIRECTION_PROPERTY_KEY)
    .set_options({ QObject::tr("Horizontal").toStdString(),
                   QObject::tr("Vertical").toStdString() })
    .set_label(QObject::tr("Direction").toStdString()).set_category(category);
}


void Mirror::draw_object(AbstractRenderer& renderer, const Style& style)
{
  assert(&renderer.scene == scene());
  auto reflection = make_reflection();
  if (reflection) { reflection->draw_recursive(renderer, style); }
}

BoundingBox Mirror::bounding_box()
{
  const auto n_children = this->n_children();
  if (is_active() && n_children > 0) {
    auto object = this->children().front();
    auto bb = object->transformation().apply(object->bounding_box());
    return bb | get_mirror_t().apply(bb);
  } else {
    return BoundingBox();
  }
}

std::string Mirror::type() const { return TYPE; }
std::unique_ptr<Object> Mirror::clone() const { return std::make_unique<Mirror>(*this); }
AbstractPropertyOwner::Flag Mirror::flags() const { return Object::flags() | Flag::Convertable; }

std::unique_ptr<Object> Mirror::make_reflection()
{
  const auto n_children = this->n_children();
  if (is_active() && n_children > 0) {
    auto reflection = this->children().front()->clone();
    reflection->set_transformation(get_mirror_t().apply(reflection->transformation()));
    return reflection;
  } else {
    return nullptr;
  }
}

std::unique_ptr<Object> Mirror::convert()
{
  std::unique_ptr<Object> converted = std::make_unique<Empty>(scene());
  copy_properties(*converted);
  copy_tags(*converted);

  converted->adopt(make_reflection());
  for (auto&& child : this->children()) {
    converted->adopt(child->clone());
  }

  return converted;
}

ObjectTransformation Mirror::get_mirror_t() const
{
  switch (property(DIRECTION_PROPERTY_KEY).value<Mirror::Direction>()) {
  case omm::Mirror::Direction::Horizontal:
    return omm::ObjectTransformation().scaled(omm::Vec2f(-1.0,  1.0));
  case omm::Mirror::Direction::Vertical:
    return omm::ObjectTransformation().scaled(omm::Vec2f( 1.0, -1.0));
  }
  Q_UNREACHABLE();
}

}  // namespace omm
