#pragma once

#include <pybind11/stl.h>
#include <string>
#include "view.h"
#include "object.h"


class Object;

class ObjectView : public View<Object>
{

public:
  ObjectView(Object& object);
  
  py::object children();
  py::object parent();
  py::object tags();
  void remove();
  py::object create_child();
  py::object create_tag();

};
