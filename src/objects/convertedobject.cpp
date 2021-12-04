#include "objects/convertedobject.h"
#include "objects/object.h"
#include "scene/disjointpathpointsetforest.h"

namespace omm
{

ConvertedObject::ConvertedObject(std::unique_ptr<Object>&& object, bool keep_children)
  : m_object(std::move(object))
  , m_keep_children(keep_children)
{
}

ConvertedObject::~ConvertedObject() = default;

std::unique_ptr<Object> ConvertedObject::extract_object()
{
  return std::move(m_object);
}

Object& ConvertedObject::object() const
{
  return *m_object;
}

bool ConvertedObject::keep_children() const
{
  return m_keep_children;
}

}  // namespace omm
