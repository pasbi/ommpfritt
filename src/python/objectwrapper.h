#pragma once

#include "python/propertyownerwrapper.h"
#include "python/wrapperfactory.h"

namespace omm
{

class ObjectWrapper : public PropertyOwnerWrapper, public WrapperFactory<Object, ObjectWrapper>
{
public:
  using PropertyOwnerWrapper::PropertyOwnerWrapper;
  py::object children() const;
  py::object parent() const;
  py::object tags() const;
  static void define_python_interface(py::object& module);
  using wrapped_type = Object;
};

}  // namespace omm
