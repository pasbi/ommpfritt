#include "aspects/propertyowner.h"

#include "external/pybind11/embed.h"
#include "external/pybind11/stl.h"

#include "python/objectwrapper.h"
#include "python/pywrapper.h"
#include "python/splinewrapper.h"
#include "python/stylewrapper.h"
#include "python/tagwrapper.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "variant.h"

namespace py = pybind11;

namespace
{
template<typename Wrapper> QString type()
{
  return QString("<class 'omm.%1'>").arg(Wrapper::wrapped_type::TYPE);
}

template<typename Head, typename... Tail>
omm::variant_type unwrap_python_object(const pybind11::object& object)
{
  if (static_cast<std::string>(py::str(object.get_type())) == type<Head>().toStdString()) {
    return &object.cast<Head>().wrapped;
  } else {
    if constexpr (sizeof...(Tail) == 0) {
      return {};
    } else {
      return unwrap_python_object<Tail...>(object);
    }
  }
}

template<std::size_t i = 0>
omm::variant_type python_to_variant(const pybind11::object& object, const omm::Type type)
{
  using T = std::decay_t<decltype(std::get<i>(omm::variant_type()))>;
  if (type == omm::get_variant_type<T>()) {
    return object.cast<T>();
  } else {
    if constexpr (i + 1 < std::variant_size_v<omm::variant_type>) {
      return python_to_variant<i + 1>(object, type);
    } else {
      return QString{"Invalid"};
    }
  }
}

template<typename VecT> VecT vector_from_pyobject(const pybind11::object& object)
{
  return VecT(object.cast<std::vector<typename VecT::element_type>>());
}

template<typename Wrapper>
typename Wrapper::wrapped_type wrapped_object_from_pyobject(const pybind11::object& object)
{
  return object.cast<Wrapper>().wrapped;
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
      return wrap(dynamic_cast<Tag&>(*owner));
    case Kind::Style:
      return wrap(dynamic_cast<Style&>(*owner));
    case Kind::Object:
      return wrap(dynamic_cast<Object&>(*owner));
    default:
      return py::none();
    }
  }
}

variant_type python_to_variant(const pybind11::object& object, const Type type)
{
  if (type == Type::String) {
    return QString::fromStdString(py::str(object));
  } else if (type == Type::Color) {
    // TODO Color-py-wrapper
    auto rgba = object.cast<std::vector<double>>();
    if (rgba.size() == 3) {
      rgba.push_back(1.0);
    }
    if (rgba.size() == 4) {
      return Color(Color::Model::RGBA, {rgba[0], rgba[1], rgba[2], rgba[3]});
    } else {
      return Color();
    }
  } else if (type == Type::FloatVector) {
    return vector_from_pyobject<Vec2f>(object);
  } else if (type == Type::IntegerVector) {
    return vector_from_pyobject<Vec2i>(object);
  } else if (type == Type::Spline || type == Type::Reference) {
    return unwrap_python_object<ObjectWrapper, TagWrapper, StyleWrapper>(object);
  } else {
    return ::python_to_variant(object, type);
  }
}

pybind11::object variant_to_python(const variant_type& variant)
{
  return std::visit(
      [](auto&& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<std::decay_t<T>, AbstractPropertyOwner*>) {
          return wrap(v);
        } else if constexpr (std::is_same_v<T, QString>) {
          return py::cast(v.toStdString());
        } else if constexpr (std::is_same_v<T, TriggerPropertyDummyValueType>) {
          return static_cast<py::object>(py::none());
        } else if constexpr (std::is_same_v<T, Vec2f> || std::is_same_v<T, Vec2i>) {
          return py::cast(v.to_stdvec());
        } else if constexpr (std::is_same_v<T, Color>) {
          return py::cast(v.components(Color::Model::RGBA));
        } else {
          return py::cast(v);
        }
      },
      variant);
}

}  // namespace omm
