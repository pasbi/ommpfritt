#pragma once


#include "view.h"
#include <string>
#include "tag.h"

namespace omm {

class TagView : public View<Tag>
{
public:
  TagView(Tag& tag);
  py::object owner();

  void remove();
};

}  // namespace omm
