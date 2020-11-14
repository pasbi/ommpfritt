#pragma once

#include "python/propertyownerwrapper.h"
#include "python/wrapperfactory.h"

namespace omm
{
class ObjectWrapper
    : public AbstractPropertyOwnerWrapper<Object>
    , public WrapperFactory<Object, ObjectWrapper>
{
public:
  using AbstractPropertyOwnerWrapper<Object>::AbstractPropertyOwnerWrapper;
  py::object children() const;
  py::object parent() const;
  py::object tags() const;
  py::object update();

  static void define_python_interface(py::object& module);
  using wrapped_type = Object;
};

}  // namespace omm
