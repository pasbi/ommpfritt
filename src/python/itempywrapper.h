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
  explicit PyWrapper();
  py::object owner() const;
  py::object type() const;

  explicit PyWrapper(Tag& tag);
  
private:
  Tag* m_wrapped;
};

class Object;
template<> class PyWrapper<Object> : public PropertyOwnerPyWrapper
{
public:
  explicit PyWrapper(Object& object);
  py::object parent() const;
  py::object children() const;
  py::object tags() const;
  py::object type() const;

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
