#include "objects/mirror.h"

#include <QObject>
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
}

}  // namespace

namespace omm
{

class Style;

Mirror::Mirror(Scene* scene) : Object(scene)
{
  add_property<OptionsProperty>(DIRECTION_PROPERTY_KEY)
    .set_options({ QObject::tr("Horizontal").toStdString(),
                   QObject::tr("Vertical").toStdString() })
    .set_label(QObject::tr("Direction").toStdString())
    .set_category(QObject::tr("Mirror").toStdString());
}


void Mirror::render(AbstractRenderer& renderer, const Style& style)
{
  renderer.set_category(AbstractRenderer::Category::Objects);
  const auto n_children = this->n_children();
  if (is_active() && n_children > 0) {
    assert(&renderer.scene == scene());
    auto mirrored = this->children().front()->clone();

    const auto direction = property(DIRECTION_PROPERTY_KEY).value<Mirror::Direction>();
    ObjectTransformation t = get_mirror_t(direction).apply(mirrored->transformation());
    mirrored->set_transformation(t);
    mirrored->render_recursive(renderer, style);
  }
}

BoundingBox Mirror::bounding_box() { return BoundingBox(); }  // TODO
std::string Mirror::type() const { return TYPE; }
std::unique_ptr<Object> Mirror::clone() const { return std::make_unique<Mirror>(*this); }
AbstractPropertyOwner::Flag Mirror::flags() const { return Object::flags() | Flag::Convertable; }

std::unique_ptr<Object> Mirror::convert()
{
  return nullptr;
}

}  // namespace omm
