#pragma once

#include "enumnames.h"
#include "geometry/vec2.h"
#include "logging.h"
#include "python/pywrapper.h"
#include <type_traits>

namespace omm
{
namespace detail
{
template<std::size_t i = 0> pybind11::object pycast_variant(const variant_type& value)
{
  using Head = std::decay_t<decltype(std::get<i>(value))>;
  if (std::holds_alternative<Head>(value)) {
    return pybind11::cast(std::get<Head>(value));
  } else {
    if constexpr (i + 1 < std::variant_size_v<variant_type>) {
      return pycast_variant<i + 1>(value);
    } else {
      return py::none();
    }
  }
}

template<typename WrappedT> py::object get_property_value(WrappedT&& wrapped, const QString& key)
{
  if (wrapped.has_property(key)) {
    const variant_type value = wrapped.property(key)->variant_value();
    if (std::holds_alternative<AbstractPropertyOwner*>(value)) {
      return wrap(std::get<AbstractPropertyOwner*>(value));
    } else if (std::holds_alternative<TriggerPropertyDummyValueType>(value)) {
      return py::none();
    } else {
      return pycast_variant(value);
    }
  } else {
    LERROR << "Failed to find property key '" << key << "'.";
    for (const QString& other_key : wrapped.properties().keys()) {
      LINFO << other_key;
    }
    return py::none();
  }
}

bool set_property_value(AbstractPropertyOwner& property_owner,
                        const QString& key,
                        const py::object& value);
}  // namespace detail

template<typename WrappedT, typename = void>
class AbstractPropertyOwnerWrapper : public PyWrapper<WrappedT>
{
  static_assert(std::is_base_of<AbstractPropertyOwner, std::decay_t<WrappedT>>::value);

public:
  using PyWrapper<WrappedT>::PyWrapper;
  [[nodiscard]] py::object get(const std::string& key) const
  {
    return detail::get_property_value(this->wrapped, QString::fromStdString(key));
  }

  [[nodiscard]] bool set(const std::string& key, const py::object& value) const
  {
    return detail::set_property_value(this->wrapped, QString::fromStdString(key), value);
  }

  [[nodiscard]] py::str str() const
  {
    std::ostringstream ostream;
    const auto* apo = &this->wrapped;
    ostream << enum_name(apo->kind, true).toStdString()
            << "[" << static_cast<const void*>(apo) << "]";
    return ostream.str();
  }

  static void define_python_interface(py::object& module)
  {
    const auto type_name = QString("Base %1").arg(WrappedT::TYPE).toUtf8().constData();
    py::class_<AbstractPropertyOwnerWrapper<WrappedT>>(module, type_name, py::dynamic_attr())
        .def("__str__", &AbstractPropertyOwnerWrapper<WrappedT>::str)
        .def("get", &AbstractPropertyOwnerWrapper<WrappedT>::get)
        .def("set", &AbstractPropertyOwnerWrapper<WrappedT>::set);
  }
  // static void add_property_shortcuts(pybind11::object& object, wrapped_type& property_owner);
};

}  // namespace omm
