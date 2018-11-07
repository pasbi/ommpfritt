#pragma once

#include <pybind11/stl.h>
#include <string>
#include "python/view.h"
#include "objects/object.h"

namespace omm {

class Object;

class ObjectView : public View<Object>
{

public:
  ObjectView(Object& object);

  py::object children();
  py::object parent();
  py::object tags();
  void remove();

};

}  // namespace omm
