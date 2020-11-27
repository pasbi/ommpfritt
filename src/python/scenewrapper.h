#pragma once

#include "python/pywrapper.h"

namespace omm
{
class SceneWrapper : public PyWrapper<Scene>
{
public:
  using PyWrapper::PyWrapper;
  template<typename T> [[nodiscard]] py::object find_items(const QString& name) const;
  static void define_python_interface(py::object& module);
};

}  // namespace omm
