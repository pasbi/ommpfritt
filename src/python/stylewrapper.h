#pragma once

#include "python/propertyownerwrapper.h"

namespace omm
{

class StyleWrapper : public PropertyOwnerWrapper
{
public:
  using PropertyOwnerWrapper::PropertyOwnerWrapper;
  using wrapped_type = Style;
  static void define_python_interface(py::object& module);
};

}  // namespace omm
