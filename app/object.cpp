#include "object.h"

#include <assert.h>
#include <algorithm>
#include <map>

#include "tag.h"
#include "scene.h"
#include "property.h"

const std::string omm::Object::TRANSFORMATION_PROPERTY_KEY = "transformation";
const std::string omm::Object::NAME_PROPERTY_KEY = "name";
const std::string omm::Object::THE_ANSWER_KEY = "ans";

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

}  // namespace

omm::Object::Object(omm::Scene& scene)
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

omm::Object::~Object()
{
}

omm::ObjectTransformation omm::Object::transformation() const
{
  return property(TRANSFORMATION_PROPERTY_KEY).cast<ObjectTransformation>().value();
}

omm::ObjectTransformation omm::Object::global_transformation() const
{
  assert(this != nullptr);

  if (is_root()) {
    return transformation();
  } else {
    return parent().global_transformation() * transformation();
  }
}

void omm::Object::set_transformation(const omm::ObjectTransformation& transformation)
{
  property(TRANSFORMATION_PROPERTY_KEY).cast<ObjectTransformation>().set_value(transformation);
}

void omm::Object::set_global_transformation(const omm::ObjectTransformation& global_transformation)
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

bool omm::Object::is_root() const
{
  return m_parent == nullptr;
}

omm::Object& omm::Object::parent() const
{
  assert(!is_root());
  return *m_parent;
}

void omm::Object::transform(const omm::ObjectTransformation& transformation)
{
  set_transformation(transformation * this->transformation());
}

omm::Scene& omm::Object::scene() const
{
  return m_scene;
}

omm::Object& omm::Object::adopt(std::unique_ptr<Object> object)
{
  assert(&object->scene() == &scene());
  assert(object->is_root());
  const ObjectTransformation gt = object->global_transformation();

  object->m_parent = this;
  Object& oref = emplace(m_children, std::move(object));
  oref.set_global_transformation(gt);
  return oref;
}

std::unique_ptr<omm::Object> omm::Object::repudiate(Object& object)
{
  const ObjectTransformation gt = object.global_transformation();
  object.m_parent = nullptr;
  std::unique_ptr<Object> optr = extract(m_children, object);
  object.set_global_transformation(gt);
  return optr;
}

omm::Tag& omm::Object::add_tag(std::unique_ptr<Tag> tag)
{
  return emplace(m_tags, std::move(tag));
}

std::unique_ptr<omm::Tag> omm::Object::remove_tag(Tag& tag)
{
  return extract(m_tags, tag);
}

void omm::Object::reset_parent(omm::Object& new_parent)
{
  assert(!is_root()); // use Object::adopt for roots.
  new_parent.adopt(m_parent->repudiate(*this));
}

omm::ObjectTransformation omm::Object::translation(const double& dx, const double dy)
{
  ObjectTransformation t = identity();
  t.at(0, 2) = dx;
  t.at(1, 2) = dy;
  return t;
}

omm::ObjectTransformation omm::Object::rotation(const double& alpha)
{
  ObjectTransformation t = identity();
  t.at(0, 0) = cos(alpha);
  t.at(1, 1) = t.at(0, 0);
  t.at(0, 1) = sin(alpha);
  t.at(1, 0) = -t.at(0, 1);
  return t;
}

omm::ObjectTransformation omm::Object::scalation(const double& sx, const double sy)
{
  ObjectTransformation t = identity();
  t.at(0, 0) = sx;
  t.at(1, 1) = sy;
  return t;
}

omm::ObjectTransformation omm::Object::identity()
{
  ObjectTransformation t;
  t.eye();
  return t;
}

nlohmann::json omm::Object::to_json() const
{
  std::vector<nlohmann::json> children;
  for (const auto& object : m_children) {
    children.push_back(object->to_json());
  }

  std::vector<nlohmann::json> tags;
  for (const auto& tag : m_tags) {
    tags.push_back(tag->to_json());
  }

  std::vector<std::unique_ptr<Object>> m_children;
  return {
    { "properties", property_map().to_json() },
    { "children", children },
    { "tags", tags },
    { "id", m_id }
  };
}

void omm::Object::update_ids() const
{
  size_t current_id = 0;
  update_ids(current_id);
}

void omm::Object::update_ids(size_t& last_id) const
{
  last_id += 1;
  m_id = last_id;

  for (const auto& child : m_children) {
    child->update_ids(last_id);
  }
}

size_t omm::Object::id() const
{
  return m_id;
}