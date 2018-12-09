#pragma once

#include <pybind11/embed.h>
#include "aspects/propertyowner.h"

namespace py = pybind11;

namespace omm
{

template<typename PropertyOwnerT>
class View
{
public:
  explicit View(PropertyOwnerT& target)
    : m_target(&target)
  {
    static_assert( std::is_base_of<AbstractPropertyOwner, PropertyOwnerT>::value,
                   "PropertyOwnerT must be derived from AbstractPropertyOwner." );
  }

  py::object get_property(const std::string& key)
  {
    throw_if_no_target();
    return m_target->property(key).get_py_object();
  }

  void set_property(const std::string& key, const py::object& value)
  {
    throw_if_no_target();
    m_target->property(key).set_py_object(value);
  }

protected:
  void throw_if_no_target()
  {
    if (m_target == nullptr) {
      throw std::runtime_error("Accessed invalid object.");
    }
  }

  PropertyOwnerT* m_target;
};

}  // namespace omm
