#pragma once

#include "tools/objectstools/objectstool.h"
#include "objects/object.h"
#include "tools/positionvariant.h"

namespace omm
{

class ObjectMoveTool : public ObjectsTool<ObjectPositions>
{
public:
  explicit ObjectMoveTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "ObjectMoveTool";
};

class PointMoveTool : public ObjectsTool<PointPositions>
{
public:
  explicit PointMoveTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "PointMoveTool";
};

}  // namespace omm
