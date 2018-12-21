#pragma once

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "aspects/propertyowner.h"
#include "scene/scene.h"
#include "tags/scripttag.h"

namespace py = pybind11;

namespace omm
{

py::object wrap(Object& object);
py::object wrap(Scene& scene);
py::object wrap(Style& stye);
py::object wrap(Tag& tag);

template<typename Ts>
py::object wrap(const Ts& items)
{
  using item_type = typename Ts::value_type;
  return py::cast(::transform<py::object, std::list>(items, [](item_type item) {
    return wrap(*item);
  }));
}

class PyWrapper
{
public:
  // it's hard to provide proper type informartion for `m_wrapped`.
  // Problem: If `PyWrapper` was a template class with template argument matching the actual type
  // of `m_wrapped`, then, e.g., `PropertyOwnerWrapper` would derive from
  // `PyWrapper<AbstractPropertyOwner>`. But what would `ObjectWrapper` derive from?
  // It could be either `PyWrapper<Object`, which would give us proper type information about
  // `m_wrapped`, or `PropertyOwnerWrapper`, which would blur the type information to the more
  // general `AbstractPropertyOwner`, but proviodes the essential methods from
  // `AbstractPropertyOwner`.
  // I also tried an approach with more than one template parameter
  // <WrappedType, ActualWrappedType, HelperTag>
  // I think it could work that way, but it is very hard to understand and to maintain.

  PyWrapper(void* wrapped) : m_wrapped(wrapped) {}
protected:
  template<typename WrappedT> WrappedT& wrapped() const
  {
    WrappedT* wrapped = static_cast<WrappedT*>(m_wrapped);
    return *wrapped;
  }

private:
  void* m_wrapped;
};

class SceneWrapper : public PyWrapper
{
public:
  using PyWrapper::PyWrapper;
  template<typename T> py::object find_items(const std::string& name) const
  {
    return wrap(wrapped<Scene>().find_items<T>(name));
  }

};

class PropertyOwnerWrapper : public PyWrapper
{
public:
  using PyWrapper::PyWrapper;
  py::object property(const std::string& key) const;
  void set(const std::string& key, const py::object& value) const;
};

class TagWrapper : public PropertyOwnerWrapper
{
public:
  using PropertyOwnerWrapper::PropertyOwnerWrapper;
  py::object owner() const;
};

class ScriptTagWrapper : public TagWrapper
{
public:
  using TagWrapper::TagWrapper;
};

class ObjectWrapper : public PropertyOwnerWrapper
{
public:
  using PropertyOwnerWrapper::PropertyOwnerWrapper;
  py::object children() const;
  py::object parent() const;
  py::object tags() const;
};

class StyleWrapper : public PropertyOwnerWrapper
{
public:
  using PropertyOwnerWrapper::PropertyOwnerWrapper;
};


}  // namespace omm


