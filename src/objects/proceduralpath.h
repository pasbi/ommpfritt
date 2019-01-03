#pragma once

#include "objects/abstractproceduralpath.h"

namespace omm
{

class Scene;

class ProceduralPath : public AbstractProceduralPath
{
public:
  explicit ProceduralPath(Scene* scene);
  std::string type() const override;
  static constexpr auto TYPE = "ProceduralPath";
  std::unique_ptr<Object> clone() const override;
  BoundingBox bounding_box() override;

  static constexpr auto CODE_PROPERTY_KEY = "code";
  static constexpr auto COUNT_PROPERTY_KEY = "count";

private:
  std::vector<Point> points() override;
  bool is_closed() const override;
};

}  // namespace omm
