#pragma once

#include "python/tagwrapper.h"
#include "tags/scripttag.h"

namespace omm
{
class ScriptTagWrapper : public TagWrapper
{
public:
  using TagWrapper::TagWrapper;
  using wrapped_type = ScriptTag;
  static void define_python_interface(py::object& module);
};

}  // namespace omm
