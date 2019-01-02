#pragma once

#include "objects/object.h"
#include "geometry/point.h"

namespace omm
{

class Scene;

class Path : public Object
{
public:
  explicit Path(Scene* scene);
  void render(AbstractRenderer& renderer, const Style& style) const override;
  BoundingBox bounding_box() const override;
  std::string type() const override;
  static constexpr auto TYPE = "Path";
  std::unique_ptr<Object> clone() const override;

private:
  std::vector<Point> m_points;
};

}  // namespace omm
