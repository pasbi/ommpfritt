#pragma once

#include "tools/itemtools/transformationtool.h"
#include "objects/object.h"
#include "tools/itemtools/positionvariant.h"

namespace omm
{

class ObjectMoveTool : public TransformationTool<ObjectPositions>
{
public:
  explicit ObjectMoveTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "ObjectMoveTool";
};

class PointMoveTool : public TransformationTool<PointPositions>
{
public:
  explicit PointMoveTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "PointMoveTool";
};

}  // namespace omm
