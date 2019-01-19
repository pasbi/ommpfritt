#include "tags/pathtag.h"
#include <QApplication>  // TODO only for icon testing
#include <QStyle>        // TODO only for icon testing
#include "properties/referenceproperty.h"
#include "properties/floatproperty.h"
#include "properties/boolproperty.h"
#include "objects/object.h"

namespace omm
{

PathTag::PathTag(Object& owner) : Tag(owner)
{
  add_property<ReferenceProperty>(PATH_REFERENCE_PROPERTY_KEY)
    .set_allowed_kinds(AbstractPropertyOwner::Kind::Object)
    .set_label("path").set_category("path");
  add_property<FloatProperty>(POSITION_PROPERTY_KEY)
    .set_label("position").set_category("path");
  add_property<BoolProperty>(ALIGN_REFERENCE_PROPERTY_KEY)
    .set_label("align").set_category("path");
}

QIcon PathTag::icon() const
{
  return QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward);
}

std::string PathTag::type() const { return TYPE; }
std::unique_ptr<Tag> PathTag::clone() const { return std::make_unique<PathTag>(*this); }

void PathTag::evaluate()
{
  auto* o = property(PATH_REFERENCE_PROPERTY_KEY).value<AbstractPropertyOwner*>();
  if (o != nullptr && o->kind() == AbstractPropertyOwner::Kind::Object) {
    auto* path_object = static_cast<Object*>(o);
    if (!path_object->is_descendant_of(owner)) {
      const double t = property(POSITION_PROPERTY_KEY).value<double>();
      const auto location = path_object->evaluate(std::clamp(t/100.0, 0.0, 1.0));
      const auto global_location = path_object->global_transformation().apply(location);

      auto transformation = owner.global_transformation();
      if (property(ALIGN_REFERENCE_PROPERTY_KEY).value<bool>()) {
        transformation.set_rotation(global_location.rotation);
      }
      transformation.set_translation(global_location.position);
      owner.set_global_transformation(transformation);

    }
  }
}

}  // namespace