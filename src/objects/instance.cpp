#include "objects/instance.h"

#include <QObject>

#include "properties/referenceproperty.h"

namespace omm
{

class Style;

Instance::Instance(Scene* scene)
  : Object(scene)
{
  add_property<ReferenceProperty>(REFERENCE_PROPERTY_KEY, AbstractPropertyOwner::Kind::Object)
    .set_label("reference").set_category("Instance");
}

void Instance::render(AbstractRenderer& renderer, const Style& style)
{
  const auto o = referenced_object();
  if (o != nullptr) {
    o->render_recursive(renderer, style);
  }
}

BoundingBox Instance::bounding_box()
{
  const auto o = referenced_object();
  if (o != nullptr) {
    return o->bounding_box();
  } else {
    return BoundingBox();
  }
}

std::string Instance::type() const
{
  return TYPE;
}

Object* Instance::referenced_object() const
{
  // Note: If you implement a cache, keep in mind that it becomes dirty if
  //  - this instance's parent changes
  //  - the referenced object changes

  if (has_reference_cycle(REFERENCE_PROPERTY_KEY)) {
    LOG(WARNING) << "detected reference cycle  "
                 << name() << ":" << property(REFERENCE_PROPERTY_KEY).label() << ".";
    return nullptr;
  } else  {
    const auto reference = property(REFERENCE_PROPERTY_KEY).value<ReferenceProperty::value_type>();
    const auto object_reference = static_cast<Object*>(reference);
    if (object_reference != nullptr && ::contains(object_reference->all_descendants(), this)) {
      LOG(WARNING) << "Instance cannot descend from referenced object.";
      return nullptr;
    } else {
      return object_reference;
    }
  }
}

std::unique_ptr<Object> Instance::clone() const
{
  return std::make_unique<Instance>(*this);
}

std::unique_ptr<Object> Instance::convert()
{
  auto* referenced_object = this->referenced_object();
  if (referenced_object != nullptr) {
    auto clone = referenced_object->clone();
    copy_properties(*clone);
    copy_tags(*clone);
    return clone;
  } else {
    return nullptr;
  }
}

Object::Flag Instance::flags() const
{
  return Object::flags() | Flag::Convertable;
}

}  // namespace omm
