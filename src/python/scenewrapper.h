#pragma once

#include "python/pywrapper.h"

namespace omm
{


class SceneWrapper : public PyWrapper<Scene>
{
public:
  using PyWrapper::PyWrapper;
  template<typename T> py::object find_items(const std::string& name) const;
  static void define_python_interface(py::object& module);
};

}  // namespace omm
