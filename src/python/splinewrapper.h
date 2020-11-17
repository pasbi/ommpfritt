#pragma once

#include "python/pywrapper.h"
#include "splinetype.h"

namespace omm
{
class SplineWrapper : public PyWrapper<SplineType>
{
public:
  SplineWrapper() = delete;
  static void define_python_interface(py::object& module);
};

}  // namespace omm
