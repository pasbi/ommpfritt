#pragma once

#include "python/propertyownerwrapper.h"
#include "python/wrapperfactory.h"
#include "tags/tag.h"

namespace omm
{
class TagWrapper
    : public AbstractPropertyOwnerWrapper<Tag>
    , public WrapperFactory<Tag, TagWrapper>
{
public:
  using AbstractPropertyOwnerWrapper<Tag>::AbstractPropertyOwnerWrapper;
  [[nodiscard]] py::object owner() const;
  static void define_python_interface(py::object& module);
};

}  // namespace omm
