#pragma once

#include <pybind11/embed.h>
#include "python/propertyownerpywrapper.h"

namespace py = pybind11;

namespace omm
{

class Tag;
class TagPyWrapper : public PropertyOwnerPyWrapper<Tag>
{
public:
  using PropertyOwnerPyWrapper<Tag>::PropertyOwnerPyWrapper;
  py::object get_owner() const;
};

}  // namespace omm
