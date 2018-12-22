#pragma once

#include <pybind11/embed.h>
#include "geometry/objecttransformation.h"

namespace omm::ObjectTransformationWrapper
{

void define_python_interface(pybind11::object& module);
using wrapped_type = ObjectTransformation;

}  // namespace omm::ObjectTransformationWrapper
