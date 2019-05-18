#include "objects/instance.h"

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
  add_property<ReferenceProperty>(REFERENCE_PROPERTY_KEY)
    .set_allowed_kinds(AbstractPropertyOwner::Kind::Object)
    .set_label(QObject::tr("reference", "Instance").toStdString()).set_category(category);
  add_property<BoolProperty>(COMBINE_STYLES_PROPERTY_KEY)
      .set_label(QObject::tr("combine styles", "Instance").toStdString()).set_category(category);
}

Instance::Instance(const Instance &other) : Object(other), m_instance(nullptr) { }

void Instance::draw_object(Painter &renderer, const Style& default_style) const
{
  if (is_active() && m_instance) {
    m_instance->set_transformation(ObjectTransformation());  // same transformation as `this`
    RenderOptions options;
    options.default_style = &default_style;
    options.always_visible = true;
    options.styles = find_styles();
    if (options.styles.empty() || property(COMBINE_STYLES_PROPERTY_KEY)->value<bool>()) {
      const auto ostyles = m_instance->find_styles();
      options.styles.reserve(options.styles.size() + ostyles.size());
      options.styles.insert(options.styles.begin(), ostyles.begin(), ostyles.end());
    }
    auto descendants = m_instance->all_descendants();
    descendants.insert(m_instance.get());
    for (auto* d : descendants) {
      for (auto *t : d->tags.items()) {
        if (t->type() == ScriptTag::TYPE) {
          static_cast<ScriptTag*>(t)->evaluate();
        }
      }
    }
    m_instance->draw_recursive(renderer, options);
  }
}

BoundingBox Instance::bounding_box() const
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

  const auto reference = property(REFERENCE_PROPERTY_KEY)->value<ReferenceProperty::value_type>();
  const auto object_reference = static_cast<Object*>(reference);
  if (object_reference != nullptr && ::contains(object_reference->all_descendants(), this)) {
    LWARNING << "Instance cannot descend from referenced object.";
    return nullptr;
  } else {
    return object_reference;
  }
}

void Instance::update()
{
  m_instance.reset();
  if (auto* referenced_object = this->referenced_object(); referenced_object != nullptr) {
    m_instance = referenced_object->clone();
    copy_properties(*m_instance);
  }
}


std::unique_ptr<Object> Instance::convert() const
{
  if (m_instance) {
    std::unique_ptr<Object> clone = m_instance->clone();
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

void Instance::post_create_hook()
{
  const auto selection = scene()->item_selection<Object>();
  if (selection.size() == 1) {
    auto* object = *selection.begin();
    using command = PropertiesCommand<ReferenceProperty>;
    scene()->submit<command>(std::set { property(REFERENCE_PROPERTY_KEY) }, object);
  }
}

}  // namespace omm
