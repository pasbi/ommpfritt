#pragma once

#include "python/propertyownerwrapper.h"
#include "renderers/style.h"

namespace omm
{
class StyleWrapper : public AbstractPropertyOwnerWrapper<Style>
{
public:
  using AbstractPropertyOwnerWrapper<Style>::AbstractPropertyOwnerWrapper;
  static void define_python_interface(py::object& module);
};

}  // namespace omm
