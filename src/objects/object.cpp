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
#include "common.h"

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
const std::string Object::THE_ANSWER_KEY = "ans";

Object::Object()
  : m_parent(nullptr)
{

  add_property( TRANSFORMATION_PROPERTY_KEY,
                std::make_unique<TransformationProperty>(Object::identity()) )
    .set_label(QObject::tr("transformation").toStdString())
    .set_category(QObject::tr("object").toStdString());

  add_property( NAME_PROPERTY_KEY,
                std::make_unique<StringProperty>("<unnamed object>") )
    .set_label(QObject::tr("Name").toStdString())
    .set_category(QObject::tr("object").toStdString());

  add_property( THE_ANSWER_KEY,
                std::make_unique<IntegerProperty>(42) )
    .set_label(QObject::tr("The Answer").toStdString())
    .set_category(QObject::tr("special").toStdString());

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
    return parent().global_transformation() * transformation();
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
      local_transformation = parent().global_transformation().i() * global_transformation;
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
  set_transformation(transformation * this->transformation());
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

ObjectTransformation Object::translation(const double& dx, const double dy)
{
  ObjectTransformation t = identity();
  t.at(0, 2) = dx;
  t.at(1, 2) = dy;
  return t;
}

ObjectTransformation Object::rotation(const double& alpha)
{
  ObjectTransformation t = identity();
  t.at(0, 0) = cos(alpha);
  t.at(1, 1) = t.at(0, 0);
  t.at(0, 1) = sin(alpha);
  t.at(1, 0) = -t.at(0, 1);
  return t;
}

ObjectTransformation Object::scalation(const double& sx, const double sy)
{
  ObjectTransformation t = identity();
  t.at(0, 0) = sx;
  t.at(1, 1) = sy;
  return t;
}

ObjectTransformation Object::identity()
{
  ObjectTransformation t;
  t.eye();
  return t;
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

std::unordered_set<HasProperties*> Object::get_selected_children_and_tags()
{
  std::unordered_set<HasProperties*> selection;

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

std::unique_ptr<Object> Object::copy() const
{
  return nullptr;
}

std::string Object::type() const
{
  return "Object";
}

void Object::register_objects()
{
#define REGISTER_OBJECT(TYPE) Object::register_type<TYPE>(#TYPE);
  REGISTER_OBJECT(Object);
#undef REGISTER_OBJECT
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


}  // namespace omm
