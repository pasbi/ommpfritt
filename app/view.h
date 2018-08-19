#pragma once

#include <pybind11/embed.h>
#include "hasproperties.h"

namespace py = pybind11;

template<typename HasPropertiesT>
class View
{
public:
  explicit View(HasPropertiesT& target)
    : m_target(&target)
  {
    static_assert(std::is_base_of<HasProperties, HasPropertiesT>::value);
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

  HasPropertiesT* m_target;
};

