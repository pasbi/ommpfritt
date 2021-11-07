#include "objects/tip.h"
#include "objects/segment.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"

namespace
{
constexpr double default_marker_size = 100.0;
constexpr auto default_marker_shape = omm::MarkerProperties::Shape::Arrow;

}  // namespace

namespace omm
{
Tip::Tip(Scene* scene)
    : Object(scene), m_marker_properties("", *this, default_marker_shape, default_marker_size)
{
  const auto tip_category = QObject::tr("Tip");
  m_marker_properties.make_properties(tip_category);
  update();
}

Tip::Tip(const Tip& other)
    : Object(other), m_marker_properties("", *this, default_marker_shape, default_marker_size)
{
}

void Tip::on_property_value_changed(Property* property)
{
  if (property == this->property(MarkerProperties::SHAPE_PROPERTY_KEY)
      || property == this->property(MarkerProperties::SIZE_PROPERTY_KEY)
      || property == this->property(MarkerProperties::ASPECT_RATIO_PROPERTY_KEY)
      || property == this->property(MarkerProperties::REVERSE_PROPERTY_KEY)) {
    update();
  } else {
    Object::on_property_value_changed(property);
  }
}

Geom::PathVector Tip::paths() const
{
  std::deque<std::unique_ptr<Point>> points;
  for (const auto& point : m_marker_properties.shape(1.0)) {
    points.push_back(std::make_unique<Point>(point));
  }
  return Geom::PathVector{Segment{std::move(points)}.to_geom_path(is_closed())};
}

}  // namespace omm
