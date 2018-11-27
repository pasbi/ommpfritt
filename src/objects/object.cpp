#include "objects/object.h"

#include <assert.h>
#include <algorithm>
#include <map>
#include <functional>
#include <QObject>

#include "tags/tag.h"
#include "properties/transformationproperty.h"
#include "properties/stringproperty.h"
#include "properties/integerproperty.h"
#include "properties/floatproperty.h"
#include "properties/referenceproperty.h"
#include "common.h"
#include "serializers/jsonserializer.h"
#include "renderers/style.h"

namespace
{

static constexpr auto CHILDREN_POINTER = "children";
static constexpr auto TYPE_POINTER = "type";

std::vector<omm::Style*> find_styles(const omm::Object& object)
{
  // TODO find style tags
  return { };
}

}  // namespace

namespace omm
{

const std::string Object::TRANSFORMATION_PROPERTY_KEY = "transformation";
const std::string Object::NAME_PROPERTY_KEY = "name";

Object::Object()
  : TreeElement(nullptr)
{

  add_property( TRANSFORMATION_PROPERTY_KEY,
                std::make_unique<TransformationProperty>(ObjectTransformation()) )
    .set_label(QObject::tr("transformation").toStdString())
    .set_category(QObject::tr("object").toStdString());

  add_property( NAME_PROPERTY_KEY,
                std::make_unique<StringProperty>("<unnamed object>") )
    .set_label(QObject::tr("Name").toStdString())
    .set_category(QObject::tr("object").toStdString());

  add_property("style_ref", std::make_unique<ReferenceProperty>())
    .set_label("style").set_category("object");
  property<ReferenceProperty>("style_ref").set_allowed_kinds(AbstractPropertyOwner::Kind::Style);

  add_property("tag_ref", std::make_unique<ReferenceProperty>())
    .set_label("tag").set_category("object");
  property<ReferenceProperty>("tag_ref").set_allowed_kinds(AbstractPropertyOwner::Kind::Tag);

  add_property("object_ref", std::make_unique<ReferenceProperty>())
    .set_label("object").set_category("object");
  property<ReferenceProperty>("object_ref").set_allowed_kinds(AbstractPropertyOwner::Kind::Object);

  add_property("any_ref", std::make_unique<ReferenceProperty>())
    .set_label("any").set_category("object");
  property<ReferenceProperty>("any_ref").set_allowed_kinds( AbstractPropertyOwner::Kind::Object
                                                          | AbstractPropertyOwner::Kind::Tag
                                                          | AbstractPropertyOwner::Kind::Style );

  // add some mockup properties for testing purposes

  // add_property( "ans",
  //               std::make_unique<IntegerProperty>(42) )
  //   .set_label(QObject::tr("The Answer").toStdString())
  //   .set_category(QObject::tr("special").toStdString());

  // add_property( "pi",
  //               std::make_unique<FloatProperty>(3.141) )
  //   .set_label(QObject::tr("pi").toStdString())
  //   .set_category(QObject::tr("object").toStdString());

  // add_property( "buddy",
  //               std::make_unique<ReferenceProperty>() )
  //   .set_label(QObject::tr("buddy").toStdString())
  //   .set_category(QObject::tr("object").toStdString());

}

Object::~Object()
{
}

ObjectTransformation Object::transformation() const
{
  return property<TransformationProperty>(TRANSFORMATION_PROPERTY_KEY).value();
}

ObjectTransformation Object::global_transformation() const
{
  assert(this != nullptr);

  if (is_root()) {
    return transformation();
  } else {
    return parent().global_transformation().apply(transformation());
  }
}

void Object::set_transformation(const ObjectTransformation& transformation)
{
  property<TransformationProperty>(TRANSFORMATION_PROPERTY_KEY).set_value(transformation);
}

void Object::set_global_transformation(const ObjectTransformation& global_transformation)
{
  ObjectTransformation local_transformation;
  if (is_root()) {
    local_transformation = global_transformation;
  } else {
    try {
      local_transformation =
        parent().global_transformation().inverted().apply(global_transformation);
    } catch (const std::runtime_error& e) {
      assert(false);
    }
  }
  set_transformation(local_transformation);
}

void Object::transform(const ObjectTransformation& transformation)
{
  set_transformation(transformation.apply(this->transformation()));
}

std::ostream& operator<<(std::ostream& ostream, const Object& object)
{
  ostream << object.type() << "[" << object.name() << "]";
  return ostream;
}

void Object::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  PropertyOwner::serialize(serializer, root);

  const auto children_key = make_pointer(root, CHILDREN_POINTER);
  serializer.start_array(n_children(), children_key);
  for (size_t i = 0; i < n_children(); ++i) {
    const auto& child = this->child(i);
    const auto child_pointer = make_pointer(children_key, i);
    serializer.set_value(child.type(), make_pointer(child_pointer, TYPE_POINTER));
    child.serialize(serializer, child_pointer);
  }
  serializer.end_array();

  // const auto tags_key = make_pointer(key, TAGS_KEY);
  // serializer.start_array(object.n_tags(), children_key);
  // for (const Object& tag : object.tags()) {
  //   tag.serialize(serializer, tags_key);
  // }
  // serializer.end_array();
}

void Object::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  PropertyOwner::deserialize(deserializer, root);

  const auto children_key = make_pointer(root, CHILDREN_POINTER);
  size_t n_children = deserializer.array_size(children_key);
  for (size_t i = 0; i < n_children; ++i) {
    const auto child_pointer = make_pointer(children_key, i);
    const auto child_type = deserializer.get_string(make_pointer(child_pointer, TYPE_POINTER));
    auto child = Object::make(child_type);
    child->deserialize(deserializer, child_pointer);
    adopt(std::move(child));
  }

  // const auto tags_key = make_pointer(key, TAGS_KEY);
  // size_t n_tags = array_size(tags_key);
  // for (size_t i = 0; i < n_tags; ++i) {
  //   object->add_tag(deserialize_tag(tags_key));
  // }
}

std::string Object::name() const
{
  return property<StringProperty>(NAME_PROPERTY_KEY).value();
}

void Object::render_recursive(AbstractRenderer& renderer, const Style& default_style) const
{
  const auto styles = find_styles(*this);
  for (const auto& style : styles) {
    render(renderer, *style);
  }
  if (styles.size() == 0) {
    render(renderer, default_style);
  }

  // if (bounding_box().intersect(renderer.bounding_box()).is_empty()) {
    for (const auto& child : children()) {
      renderer.push_transformation(child->transformation());
      child->render_recursive(renderer, default_style);
      renderer.pop_transformation();
    }
  // }
}

BoundingBox Object::recursive_bounding_box() const
{
  auto bounding_box = this->bounding_box();

  for (const auto& child : children()) {
    bounding_box = bounding_box.merge(child->recursive_bounding_box());
  }
  return transformation().apply(bounding_box);
}

}  // namespace omm
