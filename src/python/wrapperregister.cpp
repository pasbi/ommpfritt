#include "python/scenewrapper.h"
#include "python/objectwrapper.h"
#include "python/tagwrapper.h"
#include "python/propertyownerwrapper.h"
#include "python/scripttagwrapper.h"
#include "python/stylewrapper.h"
#include "python/objecttransformationwrapper.h"
#include "python/pointwrapper.h"
#include "python/pathwrapper.h"

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
  ObjectTransformationWrapper::define_python_interface(module);
  PointWrapper::define_python_interface(module);
}

}  // namespace omm
