#pragma once

#include "python/propertyownerwrapper.h"
#include "python/wrapperfactory.h"
#include "objects/object.h"

namespace omm
{
class ObjectWrapper
    : public AbstractPropertyOwnerWrapper<Object>
    , public WrapperFactory<Object, ObjectWrapper>
{
public:
  using AbstractPropertyOwnerWrapper<Object>::AbstractPropertyOwnerWrapper;
  [[nodiscard]] py::object children() const;
  [[nodiscard]] py::object parent() const;
  [[nodiscard]] py::object tags() const;
  py::object update();

  static void define_python_interface(py::object& module);
  using wrapped_type = Object;
};

}  // namespace omm
