#pragma once

#include "commands/cutpathcommand.h"

namespace omm
{

class SubdividePathCommand : public CutPathCommand
{
public:
  explicit SubdividePathCommand(PathObject& path_object);
};

}  // namespace omm
