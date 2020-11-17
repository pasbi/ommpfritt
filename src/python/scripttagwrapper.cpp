#include "python/scripttagwrapper.h"

namespace omm
{
void ScriptTagWrapper::define_python_interface(py::object& module)
{
  TagWrapper::register_wrapper<ScriptTagWrapper>();
  py::class_<ScriptTagWrapper, TagWrapper>(module, wrapped_type::TYPE);
}

}  // namespace omm
