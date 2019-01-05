#pragma once

#include "tools/objectstools/objectstool.h"
#include "objects/object.h"
#include "tools/positionvariant.h"

namespace omm
{

class ObjectScaleTool : public ObjectsTool<ObjectPositions>
{
public:
  explicit ObjectScaleTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "ObjectScaleTool";
};

class PointScaleTool : public ObjectsTool<PointPositions>
{
public:
  explicit PointScaleTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "PointScaleTool";
};

}  // namespace omm
