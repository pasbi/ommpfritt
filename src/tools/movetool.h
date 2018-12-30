#pragma once

#include "tools/objecttransformationtool.h"
#include "objects/object.h"

namespace omm
{

class MoveTool : public ObjectTransformationTool
{
public:
  explicit MoveTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "MoveTool";
};

}  // namespace omm
