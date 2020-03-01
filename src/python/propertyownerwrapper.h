#pragma once

#include "enumnames.h"
#include "python/pywrapper.h"
#include <type_traits>
#include "geometry/vec2.h"
#include "logging.h"

namespace omm
{

namespace detail
{

template<typename WrappedT>
py::object get_property_value(WrappedT&& wrapped, const QString& key)
{
  if (wrapped.has_property(key)) {
    const variant_type value = wrapped.property(key)->variant_value();
    if (std::holds_alternative<AbstractPropertyOwner*>(value)) {
      return wrap(std::get<AbstractPropertyOwner*>(value));
    } else if (std::holds_alternative<TriggerPropertyDummyValueType>(value)) {
      return py::none();
    } else if (std::holds_alternative<Vec2f>(value)) {
      return py::cast(std::get<Vec2f>(value).to_stdvec());
    } else if (std::holds_alternative<Vec2i>(value)) {
      return py::cast(std::get<Vec2i>(value).to_stdvec());
    } else if (std::holds_alternative<double>(value)) {
      return py::cast(std::get<double>(value));
    } else if (std::holds_alternative<int>(value)) {
      return py::cast(std::get<int>(value));
    } else if (std::holds_alternative<bool>(value)) {
      return py::cast(std::get<bool>(value));
    } else if (std::holds_alternative<Color>(value)) {
      return py::cast(std::get<Color>(value));
    } else {
      return py::none();
    }
  } else {
    LERROR << "Failed to find property key '" << key << "'.";
    for (const QString& key : wrapped.properties().keys()) {
      LINFO << key;
    }
    return py::none();
  }
}

bool set_property_value( AbstractPropertyOwner& property_owner,
                         const QString& key, const py::object& value );
}  // namespace detail

template<typename WrappedT, typename = void>
class AbstractPropertyOwnerWrapper : public PyWrapper<WrappedT>
{
  static_assert(std::is_base_of<AbstractPropertyOwner, std::decay_t<WrappedT>>::value);
public:
  using PyWrapper<WrappedT>::PyWrapper;
  py::object get(const std::string& key) const
  {
    return detail::get_property_value(this->wrapped, QString::fromStdString(key));
  }

  bool set(const std::string& key, const py::object& value) const
  {
    return detail::set_property_value(this->wrapped, QString::fromStdString(key), value);
  }

  py::str str() const
  {
    std::ostringstream ostream;
    const auto* apo = &this->wrapped;
    ostream << enum_name(apo->kind, true) << "[" << static_cast<const void*>(apo) << "]";
    return ostream.str();
  }

  static void define_python_interface(py::object& module)
  {
    const auto type_name = (QStringLiteral("Base") + WrappedT::TYPE).toUtf8().constData();
    py::class_<AbstractPropertyOwnerWrapper<WrappedT>>(module, type_name, py::dynamic_attr())
          .def("__str__", &AbstractPropertyOwnerWrapper<WrappedT>::str)
          .def("get", &AbstractPropertyOwnerWrapper<WrappedT>::get)
          .def("set", &AbstractPropertyOwnerWrapper<WrappedT>::set);
  }
  // static void add_property_shortcuts(pybind11::object& object, wrapped_type& property_owner);
};

}  // namespace omm
