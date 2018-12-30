#pragma once

#include "tools/objecttransformationtool.h"
#include "objects/object.h"

namespace omm
{

class ScaleTool : public ObjectTransformationTool
{
public:
  explicit ScaleTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "ScaleTool";
};

}  // namespace omm
