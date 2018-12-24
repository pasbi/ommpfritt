#pragma once

#include <memory>
#include "commands/addcommand.h"
#include "tags/tag.h"
#include "objects/object.h"

namespace omm
{

class AddTagCommand : public AddCommand<List<Tag>>
{
public:
  AddTagCommand(Object& owner, std::unique_ptr<Tag> item)
    : AddCommand(owner.tags, std::move(item))
  {
  }
};

}  // namespace omm
