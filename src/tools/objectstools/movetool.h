#pragma once

#include "tools/objectstools/objectstool.h"
#include "objects/object.h"

namespace omm
{

class MoveTool : public ObjectsTool
{
public:
  explicit MoveTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "MoveTool";
};

}  // namespace omm
