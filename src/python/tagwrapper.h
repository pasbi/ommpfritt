#pragma once

#include "python/propertyownerwrapper.h"
#include "python/wrapperfactory.h"

namespace omm
{

class TagWrapper : public PropertyOwnerWrapper, public WrapperFactory<Tag, TagWrapper>
{
public:
  using PropertyOwnerWrapper::PropertyOwnerWrapper;
  py::object owner() const;
  using wrapped_type = Tag;
  static void define_python_interface(py::object& module);
};

}  // namespace omm
