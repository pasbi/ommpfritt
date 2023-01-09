#include "objects/empty.h"
#include "properties/boolproperty.h"
#include "path/pathvector.h"

namespace omm
{
Empty::Empty(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("Empty");
  create_property<BoolProperty>(JOIN_PROPERTY_KEY, false)
      .set_label(QObject::tr("join"))
      .set_category(category);
}

BoundingBox Empty::bounding_box(const ObjectTransformation& transformation) const
{
  const Vec2f o = transformation.apply_to_position(Vec2f::o());
  return BoundingBox({o});
}

QString Empty::type() const
{
  return TYPE;
}

std::unique_ptr<PathVector> Empty::compute_geometry() const
{
  if (property(JOIN_PROPERTY_KEY)->value<bool>()) {
    return join(tree_children());
  } else {
    return std::make_unique<PathVector>();
  }
}

Flag Empty::flags() const
{
  return Flag::None;
}

}  // namespace omm
