#pragma once

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "aspects/propertyowner.h"
#include "scene/scene.h"
#include "tags/scripttag.h"

namespace py = pybind11;

namespace omm
{

template<int i> struct type;

template<typename T, typename WrappedT, typename TypeTag> class PyWrapper
{
public:
  PyWrapper(WrappedT& wrapped) : m_wrapped(&wrapped) {}
protected:
  WrappedT& wrapped() const { return *m_wrapped; }

private:
  WrappedT* m_wrapped;
};

template<> class PyWrapper<Scene, Scene, type<1>> : public PyWrapper<Scene, Scene, type<0>>
{
public:
  using PyWrapper<Scene, Scene, type<0>>::PyWrapper;
  py::object find_tags() const;
  py::object find_objects() const;
  py::object find_styles() const;

};

template<typename WrappedT> class PyWrapper<AbstractPropertyOwner, WrappedT, type<1>>
  : public PyWrapper<AbstractPropertyOwner, WrappedT, type<0>>
{
public:
  using PyWrapper<AbstractPropertyOwner, WrappedT, type<0>>::PyWrapper;
  py::object property(const std::string& key) const {
    return py::cast(this->wrapped().property(key).variant_value());
  }

  void set(const std::string& key, const py::object& value) const {
    this->wrapped().property(key).set(value.cast<Property::variant_type>());
  }
};

using AbstractPropertyOwnerWrapper
  = PyWrapper<AbstractPropertyOwner, AbstractPropertyOwner, type<1>>;

template<typename WrappedT> class PyWrapper<Tag, WrappedT, type<1>>
  : public PyWrapper<AbstractPropertyOwner, WrappedT, type<1>>
{
public:
  using PyWrapper<AbstractPropertyOwner, WrappedT, type<1>>::PyWrapper;
  py::object owner() const {
    return ObjectWrapper(tihs->wrapped().owner());
  }

};
using TagWrapper = PyWrapper<Tag, Tag, type<1>>;


template<typename WrappedT> class PyWrapper<ScriptTag, WrappedT, type<1>>
  : public PyWrapper<Tag, WrappedT, type<1>>
{
public:
  using PyWrapper<Tag, WrappedT, type<1>>::PyWrapper;
};

template<typename WrappedT> class PyWrapper<Object, WrappedT, type<1>>
  : public PyWrapper<AbstractPropertyOwner, WrappedT, type<1>>
{
public:
  using PyWrapper<AbstractPropertyOwner, WrappedT, type<1>>::PyWrapper;
  py::object children() const;
  py::object parent() const;
  py::object tags() const;
};
using ObjectWrapper = PyWrapper<Object, Object, type<1>>;

template<> class PyWrapper<Style, Style, type<1>>
  : public PyWrapper<AbstractPropertyOwner, Style, type<0>>
{
public:
  using PyWrapper<AbstractPropertyOwner, Style, type<0>>::PyWrapper;
};
using StyleWrapper = PyWrapper<Style, Style, type<1>>;


}  // namespace omm


