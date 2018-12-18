#pragma once

#include <pybind11/embed.h>
#include "python/propertyownerpywrapper.h"

namespace py = pybind11;

namespace omm
{

class Object;
class ObjectPyWrapper : public PropertyOwnerPyWrapper<Object>
{
public:
  using PropertyOwnerPyWrapper<Object>::PropertyOwnerPyWrapper;
  py::object get_parent();
  py::object get_children();
};

}  // namespace omm
