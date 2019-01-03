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
  void render(AbstractRenderer& renderer, const Style& style) override;
  BoundingBox bounding_box() override;
  std::string type() const override;
  static constexpr auto TYPE = "Path";
  std::unique_ptr<Object> clone() const override;
  void set_points(const std::vector<Point>& points);
  static constexpr auto IS_CLOSED_PROPERTY_KEY = "closed";

private:
  std::vector<Point> m_points;
};

}  // namespace omm
