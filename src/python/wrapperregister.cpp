#include "python/objectwrapper.h"
#include "python/pathwrapper.h"
#include "python/pointwrapper.h"
#include "python/pathpointwrapper.h"
#include "python/propertyownerwrapper.h"
#include "python/scenewrapper.h"
#include "python/scripttagwrapper.h"
#include "python/splinewrapper.h"
#include "python/stylewrapper.h"
#include "python/tagwrapper.h"

namespace omm
{
void register_wrappers(py::object& module)
{
  AbstractPropertyOwnerWrapper<Object>::define_python_interface(module);
  ObjectWrapper::define_python_interface(module);
  PathWrapper::define_python_interface(module);

  AbstractPropertyOwnerWrapper<Tag>::define_python_interface(module);
  TagWrapper::define_python_interface(module);
  ScriptTagWrapper::define_python_interface(module);

  AbstractPropertyOwnerWrapper<Style>::define_python_interface(module);
  StyleWrapper::define_python_interface(module);

  SceneWrapper::define_python_interface(module);
  PointWrapper::define_python_interface(module);
  PathPointWrapper::define_python_interface(module);
  SplineWrapper::define_python_interface(module);
}

}  // namespace omm
