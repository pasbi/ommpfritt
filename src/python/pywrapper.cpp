#include "aspects/propertyowner.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "python/pywrapper.h"
#include "python/objectwrapper.h"
#include "python/stylewrapper.h"
#include "python/tagwrapper.h"
#include "scene/scene.h"
#include "renderers/style.h"

namespace py = pybind11;

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

variant_type python_to_variant(const pybind11::object& object, const QString& type)
{
  if (type == "String") {
    return QString::fromStdString(py::str(object));
  } else if (type == "Bool") {
    return object.cast<bool>();
  } else if (type == "Float") {
    return object.cast<double>();
  } else if (type == "FloatVector") {
    return Vec2f(object.cast<std::vector<double>>());
  } else if (type == "IntegerVector") {
    return Vec2i(object.cast<std::vector<int>>());
  } else if (type == "Options") {
    return object.cast<std::size_t>();
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
  } else if (type == "Integer") {
    return object.cast<int>();
  } else if (type == "Reference") {
#define get_if(TYPE) \
  if (static_cast<std::string>(py::str(object.get_type())) == "<class 'omm."#TYPE"'>") { \
    return &object.cast<TYPE##Wrapper>().wrapped; \
  }

    if (object.is_none()) {
      return nullptr;
    } else get_if(Object)
    else get_if(Tag)
    else get_if(Style)
    else {
      return nullptr;
    }
#undef get_if
  } else {
    return QString("invalid");
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


