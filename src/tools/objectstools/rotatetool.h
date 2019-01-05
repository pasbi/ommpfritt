#pragma once

#include "tools/objectstools/objectstool.h"
#include "objects/object.h"
#include "tools/positionvariant.h"

namespace omm
{

class ObjectRotateTool : public ObjectsTool<ObjectPositions>
{
public:
  explicit ObjectRotateTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "ObjectRotateTool";
};

class PointRotateTool : public ObjectsTool<PointPositions>
{
public:
  explicit PointRotateTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "PointRotateTool";
};

}  // namespace omm
