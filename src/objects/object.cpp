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

namespace
{

static constexpr auto CHILDREN_POINTER = "children";
static constexpr auto TAGS_POINTER = "tags";
static constexpr auto TYPE_POINTER = "type";

std::vector<const omm::Style*> find_styles(const omm::Object& object)
{
  const auto get_style = [](const omm::Tag* tag) -> const omm::Style* {
    if (tag->type() == omm::StyleTag::TYPE) {
      constexpr auto key = omm::StyleTag::STYLE_REFERENCE_PROPERTY_KEY;
      const auto& referenceproperty= tag->property<omm::ReferenceProperty>(key);
      const auto* property_owner = referenceproperty.value();
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

const std::string Object::TRANSFORMATION_PROPERTY_KEY = "transformation";

Object::Object()
  : TreeElement(nullptr)
  , tags()
{

  add_property( TRANSFORMATION_PROPERTY_KEY,
                std::make_unique<TransformationProperty>(ObjectTransformation()) )
    .set_label(QObject::tr("transformation").toStdString())
    .set_category(QObject::tr("object").toStdString());
  add_property( "XXX", std::make_unique<ReferenceProperty>() )
    .set_label(QObject::tr("test").toStdString())
    .set_category(QObject::tr("object").toStdString());
  property<ReferenceProperty>("XXX")
    .set_allowed_kinds(AbstractPropertyOwner::Kind::All);
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
    auto child = Object::make(child_type);
    child->deserialize(deserializer, child_pointer);
    adopt(std::move(child));
  }

  const auto tags_pointer = make_pointer(root, TAGS_POINTER);
  size_t n_tags = deserializer.array_size(tags_pointer);
  std::vector<std::unique_ptr<Tag>> tags;
  tags.reserve(n_tags);
  for (size_t i = 0; i < n_tags; ++i) {
    const auto tag_pointer = make_pointer(tags_pointer, i);
    const auto tag_type = deserializer.get_string(make_pointer(tag_pointer, TYPE_POINTER));
    auto tag = Tag::make(tag_type);
    tag->deserialize(deserializer, tag_pointer);
    tag->set_owner(this);
    tags.push_back(std::move(tag));
  }
  this->tags.set(std::move(tags));
}

void Object::render_recursive(AbstractRenderer& renderer, const Style& default_style) const
{
  const auto styles = find_styles(*this);
  for (const auto* style : styles) {
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

}  // namespace omm
