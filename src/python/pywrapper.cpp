#include "aspects/propertyowner.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "python/pywrapper.h"
#include "python/objectwrapper.h"
#include "python/splinewrapper.h"
#include "python/stylewrapper.h"
#include "python/tagwrapper.h"
#include "scene/scene.h"
#include "renderers/style.h"
#include "variant.h"

namespace py = pybind11;

namespace
{

template<typename Wrapper> QString type()
{
  return QString("<class 'omm.%1'>").arg(Wrapper::wrapped_type::TYPE);
}

template<typename Head, typename... Tail>
omm::variant_type python_to_propertyowner(const pybind11::object& object)
{
  if (static_cast<std::string>(py::str(object.get_type())) == type<Head>().toStdString()) {
    return &object.cast<Head>().wrapped;
  } else {
    if constexpr (sizeof...(Tail) == 0) {
      return nullptr;
    } else {
      return python_to_propertyowner<Tail...>(object);
    }
  }
}

template<std::size_t i=0>
omm::variant_type python_to_variant(const pybind11::object& object, const QString& type)
{
  using T = std::decay_t<decltype(std::get<i>(omm::variant_type()))>;
  if (type.toStdString() == omm::variant_type_name<T>()) {
    return object.cast<T>();
  } else {
    if constexpr (i+1 < std::variant_size_v<omm::variant_type>) {
      return python_to_variant<i+1>(object, type);
    } else {
      return QString("Invalid");
    }
  }
}


}  // namespace

namespace omm
{

py::object wrap(Object& object)
{
  auto py_object = ObjectWrapper::make(object);
  // PropertyOwnerWrapper::add_property_shortcuts(py_object, *object);
  return py_object;
}

py::object wrap(Tag& tag)
{
  return TagWrapper::make(tag);
}

py::object wrap(Style& style)
{
  return py::cast(StyleWrapper(style));
}

py::object wrap(AbstractPropertyOwner* owner)
{
  if (owner == nullptr) {
    return py::none();
  } else {
    switch (owner->kind) {
    case Kind::Tag:
      return wrap(static_cast<Tag&>(*owner));
    case Kind::Style:
      return wrap(static_cast<Style&>(*owner));
    case Kind::Object:
      return wrap(static_cast<Object&>(*owner));
    default:
      return py::none();
    }
  }
}

pybind11::object wrap(SplineType& spline)
{
  return py::cast(SplineWrapper(spline));
}

variant_type python_to_variant(const pybind11::object& object, const QString& type)
{
  if (type == "String") {
    return QString::fromStdString(py::str(object));
  } else if (type == "Reference") {
    return python_to_propertyowner<ObjectWrapper, TagWrapper, StyleWrapper>(object);
  } else if (type == "Color") {
    // TODO Color-py-wrapper
    auto rgba = object.cast<std::vector<double>>();
    if (rgba.size() == 3) {
      rgba.push_back(1.0);
    }
    if (rgba.size() == 4) {
      return Color(Color::Model::RGBA, { rgba[0], rgba[1], rgba[2], rgba[3] });
    } else {
      return Color();
    }
  } else if (type == "FloatVector") {
    return Vec2f(object.cast<std::vector<double>>());
  } else if (type == "IntegerVector") {
    return Vec2i(object.cast<std::vector<int>>());
  } else {
    return ::python_to_variant(object, type);
  }
}

pybind11::object variant_to_python(variant_type variant)
{
  return std::visit([](auto&& v) {
    using T = std::decay_t<decltype (v)>;
    if constexpr (std::is_same_v<AbstractPropertyOwner*, std::decay_t<T>>) {
      return wrap(v);
    } else if constexpr (std::is_same_v<T, QString>) {
      return py::cast(v.toStdString());
    } else if constexpr (std::is_same_v<T, TriggerPropertyDummyValueType>) {
      return static_cast<py::object>(py::none());
    } else if constexpr (std::is_same_v<T, Vec2f>) {
      return py::cast(v.to_stdvec());
    } else if constexpr (std::is_same_v<T, Vec2i>) {
      return py::cast(v.to_stdvec());
    } else if constexpr (std::is_same_v<T, Color>) {
      return py::cast(v.components(Color::Model::RGBA));
    } else {
      return py::cast(v);
    }
  }, variant);
}


}  // namespace omm


