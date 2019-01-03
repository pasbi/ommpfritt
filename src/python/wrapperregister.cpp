#include "python/scenewrapper.h"
#include "python/objectwrapper.h"
#include "python/tagwrapper.h"
#include "python/propertyownerwrapper.h"
#include "python/scripttagwrapper.h"
#include "python/stylewrapper.h"
#include "python/objecttransformationwrapper.h"
#include "python/pointwrapper.h"

namespace omm
{

void register_wrappers(py::object& module)
{
  PropertyOwnerWrapper::define_python_interface(module);
  TagWrapper::define_python_interface(module);
  ScriptTagWrapper::define_python_interface(module);
  ObjectWrapper::define_python_interface(module);
  StyleWrapper::define_python_interface(module);
  SceneWrapper::define_python_interface(module);
  ObjectTransformationWrapper::define_python_interface(module);
  PointWrapper::define_python_interface(module);
}

}  // namespace omm
