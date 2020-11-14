#include "python/stylewrapper.h"
#include "renderers/style.h"

namespace omm
{
void StyleWrapper::define_python_interface(py::object& module)
{
  py::class_<StyleWrapper, AbstractPropertyOwnerWrapper<Style>>(module, wrapped_type::TYPE);
}

}  // namespace omm
