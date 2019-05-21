#pragma once

#include "properties/propertygroups/propertygroup.h"
#include "renderers/painter.h"

namespace omm
{

class MarkerProperties : public PropertyGroup
{
public:
  enum class Shape { None, Arrow, Bar, Circle, Diamond };
  MarkerProperties(const std::string& prefix, AbstractPropertyOwner& property_owner,
                   const Shape default_shape, const double default_size);
  void make_properties(const std::string &category) const override;

  void draw_marker(Painter& painter, const Point& location,
                   const Color &color, const double width) const;

  static constexpr auto SHAPE_PROPERTY_KEY = "shape";
  static constexpr auto SIZE_PROPERTY_KEY = "size";
  static constexpr auto ASPECT_RATIO_PROPERTY_KEY = "aspectratio";
  static constexpr auto REVERSE_PROPERTY_KEY = "reverse";

  std::vector<std::string> shapes() const;
  std::vector<Point> shape(const double width) const;
  std::vector<Point> arrow(const Vec2f& size) const;
  std::vector<Point> bar(const Vec2f& size) const;
  std::vector<Point> circle(const Vec2f& size) const;
  std::vector<Point> diamond(const Vec2f& size) const;

private:
  const Shape m_default_shape;
  const double m_default_size;
};

}  // namespace
