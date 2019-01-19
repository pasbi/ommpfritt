#pragma once

#include "objects/object.h"
#include "geometry/point.h"
#include "geometry/cubic.h"

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
  std::vector<Point*> points();
  void set_points(const std::vector<Point>& points);
  static constexpr auto IS_CLOSED_PROPERTY_KEY = "closed";
  static constexpr auto POINTS_POINTER = "points";
  static constexpr auto INTERPOLATION_PROPERTY_KEY = "interpolation";

  OrientedPoint evaluate(const double t) override;
  double path_length() override;

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  bool tangents_modifiable() const;

  enum class InterpolationMode { Linear, Smooth, Bezier };
  void deselect_all_points();

  std::map<Point*, Point>
  modified_points(const bool constrain_to_selection, InterpolationMode mode);
  bool is_closed() const;

private:
  std::vector<Point> m_points;
};

}  // namespace omm
