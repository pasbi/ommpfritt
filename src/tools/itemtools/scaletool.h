#pragma once

#include "tools/itemtools/transformationtool.h"
#include "objects/object.h"
#include "tools/itemtools/positionvariant.h"

namespace omm
{

class ObjectScaleTool : public TransformationTool<ObjectPositions>
{
public:
  explicit ObjectScaleTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "ObjectScaleTool";
};

class PointScaleTool : public TransformationTool<PointPositions>
{
public:
  explicit PointScaleTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "PointScaleTool";
};

}  // namespace omm
