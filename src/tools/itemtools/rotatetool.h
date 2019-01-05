#pragma once

#include "tools/itemtools/transformationtool.h"
#include "objects/object.h"
#include "tools/itemtools/positionvariant.h"

namespace omm
{

class ObjectRotateTool : public TransformationTool<ObjectPositions>
{
public:
  explicit ObjectRotateTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "ObjectRotateTool";
};

class PointRotateTool : public TransformationTool<PointPositions>
{
public:
  explicit PointRotateTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "PointRotateTool";
};

}  // namespace omm
