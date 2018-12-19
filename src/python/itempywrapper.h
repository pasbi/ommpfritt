#pragma once

#include <pybind11/embed.h>
#include "python/propertyownerpywrapper.h"

namespace py = pybind11;

namespace omm
{

template<typename T> class PyWrapper;

class Tag;
template<> class PyWrapper<Tag> : public PropertyOwnerPyWrapper
{
public:
  explicit PyWrapper(Tag& tag);
  py::object get_owner() const;
  
private:
  Tag* m_wrapped;
};

class Object;
template<> class PyWrapper<Object> : public PropertyOwnerPyWrapper
{
public:
  explicit PyWrapper(Object& object);
  py::object get_parent() const;
  py::object get_children() const;
  py::object get_tags() const;

private:
  Object* m_wrapped;
};

class Style;
template<> class PyWrapper<Style> : public PropertyOwnerPyWrapper
{
public:
  explicit PyWrapper(Style& object);

private:
  Style* m_wrapped;
};

}  // namespace omm
