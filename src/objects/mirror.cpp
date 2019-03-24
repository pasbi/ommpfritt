#include "objects/mirror.h"

#include <QObject>
#include "objects/empty.h"
#include "properties/optionsproperty.h"

namespace
{

omm::ObjectTransformation get_mirror_t(omm::Mirror::Direction direction)
{
  switch (direction) {
  case omm::Mirror::Direction::Horizontal:
    return omm::ObjectTransformation().scaled(arma::vec2{ -1.0,  1.0 });
  case omm::Mirror::Direction::Vertical:
    return omm::ObjectTransformation().scaled(arma::vec2{  1.0, -1.0 });
  }
  Q_UNREACHABLE();
}

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


void Mirror::render(AbstractRenderer& renderer, const Style& style)
{
  renderer.set_category(AbstractRenderer::Category::Objects);
  assert(&renderer.scene == scene());
  auto reflection = make_reflection();
  if (reflection) { reflection->render_recursive(renderer, style); }
}

BoundingBox Mirror::bounding_box() { return BoundingBox(); }  // TODO
std::string Mirror::type() const { return TYPE; }
std::unique_ptr<Object> Mirror::clone() const { return std::make_unique<Mirror>(*this); }
AbstractPropertyOwner::Flag Mirror::flags() const { return Object::flags() | Flag::Convertable; }

std::unique_ptr<Object> Mirror::make_reflection()
{
  const auto n_children = this->n_children();
  if (is_active() && n_children > 0) {
    auto reflection = this->children().front()->clone();
    const auto direction = property(DIRECTION_PROPERTY_KEY).value<Mirror::Direction>();
    ObjectTransformation t = get_mirror_t(direction).apply(reflection->transformation());
    reflection->set_transformation(t);
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

}  // namespace omm
