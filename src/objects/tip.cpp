#include "objects/tip.h"
#include "objects/path/segment.h"
#include "objects/path/pathpoint.h"
#include "objects/path/enhancedpathvector.h"
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

EnhancedPathVector Tip::paths() const
{
  auto points = m_marker_properties.shape(1.0);
  EnhancedPathVector::JoinedPointIndices joined_points{{{0, points.size() - 1}}};
  return {Geom::PathVector{Segment{std::move(points)}.to_geom_path()}, joined_points};
}

}  // namespace omm
