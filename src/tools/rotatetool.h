#pragma once

#include "tools/objecttransformationtool.h"
#include "objects/object.h"

namespace omm
{

class RotateTool : public ObjectTransformationTool
{
public:
  explicit RotateTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "RotateTool";
};

}  // namespace omm
