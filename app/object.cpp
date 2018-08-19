#include "object.h"

#include <assert.h>
#include <algorithm>

#include "tag.h"
#include "scene.h"
#include "property.h"

const std::string Object::TRANSFORMATION_PROPERTY_KEY = "transformation";
const std::string Object::NAME_PROPERTY_KEY = "name";
const std::string Object::THE_ANSWER_KEY = "ans";


namespace {
template<typename T> 
T& emplace(std::vector<std::unique_ptr<T>>& container, std::unique_ptr<T> obj)
{
  T& ref = *obj;
  container.push_back(std::move(obj));
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

}

Object::Object(Scene& scene)
  : m_parent(nullptr)
  , m_scene(scene)
{
  add_property( TRANSFORMATION_PROPERTY_KEY,
                std::make_unique<TransformationProperty>(Object::identity()) );
  add_property( NAME_PROPERTY_KEY,
                std::make_unique<StringProperty>("<Unnamed Object>") );
  add_property( THE_ANSWER_KEY,
                std::make_unique<IntegerProperty>(42) );
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

Scene& Object::scene() const
{
  return m_scene;
}

Object& Object::adopt(std::unique_ptr<Object> object)
{
  assert(&object->scene() == &scene());
  assert(object->is_root());
  const ObjectTransformation gt = object->global_transformation();

  object->m_parent = this;
  Object& oref = emplace(m_children, std::move(object));
  oref.set_global_transformation(gt);
  return oref;
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
  return emplace(m_tags, std::move(tag));
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

ObjectTransformation Object::translation(const Float& dx, const Float dy)
{
  ObjectTransformation t = identity();
  t.at(0, 2) = dx;
  t.at(1, 2) = dy;
  return t;
}

ObjectTransformation Object::rotation(const Float& alpha)
{
  ObjectTransformation t = identity();
  t.at(0, 0) = cos(alpha);
  t.at(1, 1) = t.at(0, 0);
  t.at(0, 1) = sin(alpha);
  t.at(1, 0) = -t.at(0, 1);
  return t;
}

ObjectTransformation Object::scalation(const Float& sx, const Float sy)
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
