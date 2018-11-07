#pragma once


#include "python/view.h"
#include <string>
#include "tags/tag.h"

namespace omm
{

class TagView : public View<Tag>
{
public:
  TagView(Tag& tag);
};

}  // namespace omm
