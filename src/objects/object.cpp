#include "objects/object.h"

#include <cassert>
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
#include "tags/styletag.h"
#include "scene/contextes.h"

namespace
{

static constexpr auto CHILDREN_POINTER = "children";
static constexpr auto TAGS_POINTER = "tags";
static constexpr auto TYPE_POINTER = "type";

std::vector<const omm::Style*> find_styles(const omm::Object& object)
{
  const auto get_style = [](const omm::Tag* tag) -> const omm::Style* {
    if (tag->type() == omm::StyleTag::TYPE) {
      const auto* property_owner = tag->property(omm::StyleTag::STYLE_REFERENCE_PROPERTY_KEY)
                                       .value<omm::ReferenceProperty::value_type>();
      assert(  property_owner == nullptr
            || property_owner->kind() == omm::AbstractPropertyOwner::Kind::Style );
      return static_cast<const omm::Style*>(property_owner);
    } else {
      return nullptr;
    }
  };

  const auto tags = object.tags.ordered_items();
  return ::filter_if(::transform<const omm::Style*>(tags, get_style), ::is_not_null);
}

}  // namespace

namespace omm
{

Object::Object(Scene* scene)
  : TreeElement(nullptr)
  , tags(scene)
{
  set_scene(scene);

  add_property<StringProperty>(NAME_PROPERTY_KEY, "<unnamed object>")
    .set_label(QObject::tr("Name").toStdString())
    .set_category(QObject::tr("basic").toStdString());

  add_property<TransformationProperty>(TRANSFORMATION_PROPERTY_KEY)
    .set_label(QObject::tr("transformation").toStdString())
    .set_category(QObject::tr("object").toStdString());
}

Object::Object(const Object& other)
  : PropertyOwner(other)
  , TreeElement(other)
  , tags(other.tags)
  , m_draw_children(other.m_draw_children)
{
  set_scene(other.m_scene);
}

Object::~Object()
{
}

void Object::set_scene(Scene* scene)
{
  tags.set_scene(scene);
  m_scene = scene;
}

ObjectTransformation Object::transformation() const
{
  return property(TRANSFORMATION_PROPERTY_KEY).value<ObjectTransformation>();
}

ObjectTransformation Object::global_transformation() const
{
  if (is_root()) {
    return transformation();
  } else {
    return parent().global_transformation().apply(transformation());
  }
}

void Object::set_transformation(const ObjectTransformation& transformation)
{
  property(TRANSFORMATION_PROPERTY_KEY).set(transformation);
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

  const auto children_pointer = make_pointer(root, CHILDREN_POINTER);
  serializer.start_array(n_children(), children_pointer);
  for (size_t i = 0; i < n_children(); ++i) {
    const auto& child = this->child(i);
    const auto child_pointer = make_pointer(children_pointer, i);
    serializer.set_value(child.type(), make_pointer(child_pointer, TYPE_POINTER));
    child.serialize(serializer, child_pointer);
  }
  serializer.end_array();

  const auto tags_pointer = make_pointer(root, TAGS_POINTER);
  serializer.start_array(tags.size(), tags_pointer);
  for (size_t i = 0; i < tags.size(); ++i) {
    const auto& tag = tags.item(i);
    const auto tag_pointer = make_pointer(tags_pointer, i);
    serializer.set_value(tag.type(), make_pointer(tag_pointer, TYPE_POINTER));
    tag.serialize(serializer, tag_pointer);
  }
  serializer.end_array();
}

void Object::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  PropertyOwner::deserialize(deserializer, root);

  const auto children_pointer = make_pointer(root, CHILDREN_POINTER);
  size_t n_children = deserializer.array_size(children_pointer);
  for (size_t i = 0; i < n_children; ++i) {
    const auto child_pointer = make_pointer(children_pointer, i);
    const auto child_type = deserializer.get_string(make_pointer(child_pointer, TYPE_POINTER));
    try {
      auto child = Object::make(child_type, static_cast<Scene*>(m_scene));
      child->deserialize(deserializer, child_pointer);
      adopt(std::move(child));
    } catch (std::out_of_range& e) {
      const auto message = "Failed to retrieve object type '" + child_type + "'.";
      LOG(ERROR) << message;
      throw AbstractDeserializer::DeserializeError(message);
    }
  }

  const auto tags_pointer = make_pointer(root, TAGS_POINTER);
  size_t n_tags = deserializer.array_size(tags_pointer);
  std::vector<std::unique_ptr<Tag>> tags;
  tags.reserve(n_tags);
  for (size_t i = 0; i < n_tags; ++i) {
    const auto tag_pointer = make_pointer(tags_pointer, i);
    const auto tag_type = deserializer.get_string(make_pointer(tag_pointer, TYPE_POINTER));
    auto tag = Tag::make(tag_type, *this);
    tag->deserialize(deserializer, tag_pointer);
    tags.push_back(std::move(tag));
  }
  this->tags.set(std::move(tags));
}

void Object::render_recursive(AbstractRenderer& renderer, const Style& default_style)
{
  const auto styles = find_styles(*this);
  for (const auto* style : styles) {
    render(renderer, *style);
  }
  if (styles.size() == 0) {
    render(renderer, default_style);
  }

  // if (bounding_box().intersect(renderer.bounding_box()).is_empty()) {
  if (m_draw_children) {
    for (const auto& child : children()) {
      renderer.push_transformation(child->transformation());
      child->render_recursive(renderer, default_style);
      renderer.pop_transformation();
    }
  }
  // }
}

BoundingBox Object::recursive_bounding_box()
{
  auto bounding_box = this->bounding_box();

  for (const auto& child : children()) {
    bounding_box = bounding_box.merge(child->recursive_bounding_box());
  }
  return transformation().apply(bounding_box);
}

std::unique_ptr<AbstractRAIIGuard> Object::acquire_set_parent_guard()
{
  class SetParentGuard : public AbstractRAIIGuard
  {
  public:
    explicit SetParentGuard(Object& self)
      : m_self(self), m_global_transformation(m_self.global_transformation()) { }

    ~SetParentGuard() { m_self.set_global_transformation(m_global_transformation); }

  private:
    Object& m_self;
    const ObjectTransformation m_global_transformation;
  };
  return std::make_unique<SetParentGuard>(*this);
}

std::unique_ptr<Object> Object::clone(Scene* scene) const
{
  auto clone = this->clone();
  clone->m_scene = scene;
  return clone;
}

std::unique_ptr<Object> Object::convert()
{
  return clone();
}

Object::Flag Object::flags() const
{
  return Flag::None;
}

Scene* Object::scene() const
{
  return m_scene;
}

void Object::copy_properties(Object& other) const
{
  const auto keys = [](const Object& o) {
    return ::transform<std::string, std::set>(o.properties().keys(), ::identity);
  };

  for (const auto& key : ::intersect(keys(other), keys(*this))) {
    const auto& p = property(key);
    auto& other_property = other.property(key);
    if (other_property.is_compatible(p)) {
      other_property.set(p.variant_value());
    }
  }
}

void Object::copy_tags(Object& other) const
{
  for (const Tag* tag : tags.items()) {
    ListOwningContext<Tag> context(tag->clone(), other.tags);
    other.tags.insert(context);
  }
}

}  // namespace omm
