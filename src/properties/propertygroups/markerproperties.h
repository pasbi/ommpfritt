#pragma once

#include "properties/propertygroups/propertygroup.h"
#include <vector>

namespace omm
{

class Painter;
class Point;

class MarkerProperties : public PropertyGroup
{
public:
  enum class Shape { None, Arrow, Bar, Circle, Diamond };
  MarkerProperties(const QString& prefix,
                   AbstractPropertyOwner& property_owner,
                   Shape default_shape,
                   double default_size);
  void make_properties(const QString& category) const override;

  void draw_marker(Painter& painter, const Point& location, const Color& color, double width) const;

  static constexpr auto SHAPE_PROPERTY_KEY = "shape";
  static constexpr auto SIZE_PROPERTY_KEY = "size";
  static constexpr auto ASPECT_RATIO_PROPERTY_KEY = "aspectratio";
  static constexpr auto REVERSE_PROPERTY_KEY = "reverse";

  static std::deque<QString> shapes();
  std::vector<Point> shape(double width) const;
  static std::vector<Point> arrow(const Vec2f& size);
  static std::vector<Point> bar(const Vec2f& size);
  static std::vector<Point> circle(const Vec2f& size);
  static std::vector<Point> diamond(const Vec2f& size);

private:
  const Shape m_default_shape;
  const double m_default_size;
};

}  // namespace omm
