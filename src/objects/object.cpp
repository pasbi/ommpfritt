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

template<typename T>
T& insert(std::vector<std::unique_ptr<T>>& container, std::unique_ptr<T> obj, size_t pos)
{
  auto pos_it = container.begin();
  std::advance(pos_it, pos);

  T& ref = *obj;
  assert(obj.get() != nullptr);
  container.insert(pos_it, std::move(obj));
  return ref;
}

template<typename T>
std::unique_ptr<T> extract(std::vector<std::unique_ptr<T>>& container, T& obj)
{
  const auto it = std::find_if( std::begin(container),
                                std::end(container),
                                [&obj](const std::unique_ptr<T>& a) {
    return a.get() == &obj;
  });
  assert(it != std::end(container));
  std::unique_ptr<T> uptr = std::move(*it);
  container.erase(it);
  return std::move(uptr);
}

}  // namespace

namespace omm
{

const std::string Object::TRANSFORMATION_PROPERTY_KEY = "transformation";
const std::string Object::NAME_PROPERTY_KEY = "name";

Object::Object()
  : m_parent(nullptr)
{

  add_property( TRANSFORMATION_PROPERTY_KEY,
                std::make_unique<TransformationProperty>(ObjectTransformation()) )
    .set_label(QObject::tr("transformation").toStdString())
    .set_category(QObject::tr("object").toStdString());

  add_property( NAME_PROPERTY_KEY,
                std::make_unique<StringProperty>("<unnamed object>") )
    .set_label(QObject::tr("Name").toStdString())
    .set_category(QObject::tr("object").toStdString());

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
  return property(TRANSFORMATION_PROPERTY_KEY).cast<ObjectTransformation>().value();
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
  property(TRANSFORMATION_PROPERTY_KEY).cast<ObjectTransformation>().set_value(transformation);
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

bool Object::is_root() const
{
  return m_parent == nullptr;
}

Object& Object::parent() const
{
  assert(!is_root());
  return *m_parent;
}

void Object::transform(const ObjectTransformation& transformation)
{
  set_transformation(transformation.apply(this->transformation()));
}

Object& Object::adopt(std::unique_ptr<Object> object, const Object* predecessor)
{
  assert(object->is_root());
  const ObjectTransformation gt = object->global_transformation();
  assert(predecessor == nullptr || &predecessor->parent() == this);
  const size_t pos = get_insert_position(predecessor);

  object->m_parent = this;
  Object& oref = insert(m_children, std::move(object), pos);
  oref.set_global_transformation(gt);
  return oref;
}

Object& Object::adopt(std::unique_ptr<Object> object)
{
  const size_t n = n_children();
  const Object* predecessor = n == 0 ? nullptr : &child(n-1);
  return adopt(std::move(object), predecessor);
}

std::unique_ptr<Object> Object::repudiate(Object& object)
{
  const ObjectTransformation gt = object.global_transformation();
  object.m_parent = nullptr;
  std::unique_ptr<Object> optr = extract(m_children, object);
  object.set_global_transformation(gt);
  return optr;
}

Tag& Object::add_tag(std::unique_ptr<Tag> tag)
{
  return insert(m_tags, std::move(tag), m_tags.size());
}

std::unique_ptr<Tag> Object::remove_tag(Tag& tag)
{
  return extract(m_tags, tag);
}

void Object::reset_parent(Object& new_parent)
{
  assert(!is_root()); // use Object::adopt for roots.
  new_parent.adopt(m_parent->repudiate(*this));
}

ObjectRefs Object::children() const
{
  static const auto f = [](const std::unique_ptr<Object>& uptr) {
    return ObjectRef(*uptr);
  };
  return ::transform<ObjectRef>(m_children, f);
}

size_t Object::n_children() const
{
  return m_children.size();
}

Object& Object::child(size_t i) const
{
  return *m_children[i];
}

std::set<HasProperties*> Object::get_selected_children_and_tags()
{
  std::set<HasProperties*> selection;

  if (is_selected()) {
    selection.insert(this);
  }

  for (auto& tag : m_tags) {
    if (tag->is_selected()) {
      selection.insert(tag.get());
    }
  }

  for (auto& child : m_children) {
    const auto child_selection = child->get_selected_children_and_tags();
    selection.insert(child_selection.begin(), child_selection.end());
  }

  return selection;
}

size_t Object::row() const
{
  assert (!is_root());
  const auto siblings = parent().children();
  for (size_t i = 0; i < siblings.size(); ++i) {
    if (&siblings[i].get() == this) {
      return i;
    }
  }
  assert(false);
}

const Object* Object::predecessor() const
{
  assert(!is_root());
  const auto pos = row();
  if (pos == 0) {
    return nullptr;
  } else {
    return &parent().child(pos - 1);
  }
}

size_t Object::get_insert_position(const Object* child_before_position) const
{
  if (child_before_position == nullptr) {
    return 0;
  } else {
    assert(&child_before_position->parent() == this);
    return child_before_position->row() + 1;
  }
}

std::ostream& operator<<(std::ostream& ostream, const Object& object)
{
  ostream << object.type()
          << "[" << object.property<std::string>(Object::NAME_PROPERTY_KEY).value() << "]";
  return ostream;
}

void Object::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  HasProperties::serialize(serializer, root);

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
  HasProperties::deserialize(deserializer, root);

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

std::unique_ptr<Object> Object::copy() const
{
  return Serializable::copy<Object, JSONSerializer, JSONDeserializer>(Object::make(type()), *this);
}

std::string Object::name() const
{
  return property<std::string>(NAME_PROPERTY_KEY).value();
}

void Object::render_recursive(AbstractRenderer& renderer) const
{
  render(renderer);
  // if (bounding_box().intersect(renderer.bounding_box()).is_empty()) {
    for (const auto& child : m_children) {
      renderer.push_transformation(child->transformation());
      child->render_recursive(renderer);
      renderer.pop_transformation();
    }
  // }
}

BoundingBox Object::recursive_bounding_box() const
{
  auto bounding_box = this->bounding_box();

  for (const auto& child : m_children) {
    bounding_box = bounding_box.merge(child->recursive_bounding_box());
  }
  return transformation().apply(bounding_box);
}

Style Object::style() const
{
  // collect style from style tags
  // style tags can be combined (e.g., enable stroke in style A and fill in style B.
  // combine them to have both stroke and fill.
  // if there is no style tag, return "default" style (maybe only simple stroke).

  return Style(); // TODO
}

}  // namespace omm
