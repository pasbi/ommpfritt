#include "objects/instance.h"

#include <QObject>

#include "properties/referenceproperty.h"
#include "properties/boolproperty.h"

namespace omm
{

class Style;

Instance::Instance(Scene* scene)
  : Object(scene)
{
  add_property<ReferenceProperty>(REFERENCE_PROPERTY_KEY)
    .set_allowed_kinds(AbstractPropertyOwner::Kind::Object)
    .set_label(QObject::tr("reference", "Instance").toStdString())
    .set_category(QObject::tr("Instance", "Instance").toStdString());
  add_property<BoolProperty>(COMBINE_STYLES_PROPERTY_KEY)
    .set_label(QObject::tr("combine styles", "Instance").toStdString())
    .set_category(QObject::tr("Instance", "Instance").toStdString());
}

void Instance::render(AbstractRenderer& renderer, const Style& default_style)
{
  if (is_active()) {
    const auto o = referenced_object();
    if (o != nullptr) {
      RenderOptions options;
      options.default_style = &default_style;
      options.always_visible = true;
      options.styles = find_styles();
      if (options.styles.empty() || property(COMBINE_STYLES_PROPERTY_KEY).value<bool>()) {
        const auto ostyles = o->find_styles();
        options.styles.reserve(options.styles.size() + ostyles.size());
        options.styles.insert(options.styles.begin(), ostyles.begin(), ostyles.end());
      }
      o->render_recursive(renderer, options);
    }
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

std::string Instance::type() const { return TYPE; }
std::unique_ptr<Object> Instance::clone() const { return std::make_unique<Instance>(*this); }
Object::Flag Instance::flags() const { return Object::flags() | Flag::Convertable; }

}  // namespace omm
