#pragma once

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include "aspects/propertyowner.h"
#include "python/itempywrapper.h"

namespace py = pybind11;

namespace omm
{

class ScenePyWrapper
{
public:
  explicit ScenePyWrapper(Scene& wrapped);
  template<typename T> py::object find_items(const std::string& name);

private:
  Scene* m_wrapped;
};

}  // namespace omm
