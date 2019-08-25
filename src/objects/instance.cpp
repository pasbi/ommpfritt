#include "objects/instance.h"

#include "objects/empty.h"
#include <QObject>
#include "scene/scene.h"
#include "properties/referenceproperty.h"
#include "properties/boolproperty.h"
#include "commands/propertycommand.h"
#include "tags/tag.h"
#include "tags/scripttag.h"

namespace omm
{

class Style;

Instance::Instance(Scene* scene)
  : Object(scene)
{
  static const auto category = QObject::tr("Instance").toStdString();
  create_property<ReferenceProperty>(REFERENCE_PROPERTY_KEY)
    .set_allowed_kinds(AbstractPropertyOwner::Kind::Object)
    .set_label(QObject::tr("reference", "Instance").toStdString()).set_category(category);
  create_property<BoolProperty>(COMBINE_STYLES_PROPERTY_KEY)
    .set_label(QObject::tr("combine styles", "Instance").toStdString()).set_category(category);
  update();
}

Instance::Instance(const Instance &other) : Object(other) {}

void Instance::draw_object(Painter &renderer, const Style& default_style) const
{
  if (is_active()) {
    const auto* reference = referenced_object();
    ReferenceDepthGuard guard(renderer);
    if (reference != nullptr && guard) {
      renderer.push_transformation(reference->global_transformation(true).inverted());
      reference->draw_recursive(renderer, default_style);
      renderer.pop_transformation();
    }
  }
}

BoundingBox Instance::bounding_box(const ObjectTransformation &transformation) const
{
  if (is_active()) {
    const auto* reference = referenced_object();
    if (reference != nullptr) {
      return reference->recursive_bounding_box(transformation);
    } else {
      return BoundingBox();
    }
  } else {
    return BoundingBox();
  }
}

Object* Instance::referenced_object() const
{
  // Note: If you implement a cache, keep in mind that it becomes dirty if
  //  - this instance's parent changes
  //  - the referenced object changes

  const auto reference = property(REFERENCE_PROPERTY_KEY)->value<ReferenceProperty::value_type>();
  const auto object_reference = static_cast<Object*>(reference);
  if (object_reference != nullptr && ::contains(object_reference->all_descendants(), this)) {
    LWARNING << "Instance cannot descend from referenced object.";
    return nullptr;
  } else {
    return object_reference;
  }
}

std::unique_ptr<Object> Instance::convert() const
{
  std::unique_ptr<Object> clone;
  if (is_active()) {
    const auto* reference = referenced_object();
    if (reference != nullptr) {
      clone = reference->clone();
      copy_properties(*clone);
      copy_tags(*clone);
    }
  }
  if (!clone) {
    clone = std::make_unique<Empty>(scene());
  }
  return clone;
}

std::string Instance::type() const { return TYPE; }
std::unique_ptr<Object> Instance::clone() const { return std::make_unique<Instance>(*this); }
Object::Flag Instance::flags() const { return Object::flags() | Flag::Convertable; }

void Instance::post_create_hook()
{
  const auto selection = scene()->item_selection<Object>();
  if (selection.size() == 1) {
    auto* object = *selection.begin();
    using command = PropertiesCommand<ReferenceProperty>;
    scene()->submit<command>(std::set { property(REFERENCE_PROPERTY_KEY) }, object);
  }
}

void Instance::on_property_value_changed(Property *property)
{
  if (   property == this->property(REFERENCE_PROPERTY_KEY)
      || property == this->property(COMBINE_STYLES_PROPERTY_KEY)) {
    LINFO << "reference property changed";
  } else {
    Object::on_property_value_changed(property);
  }
}

}  // namespace omm
