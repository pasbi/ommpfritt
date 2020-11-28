#include "tags/pathtag.h"
#include "objects/object.h"
#include "properties/floatproperty.h"

namespace omm
{
PathTag::PathTag(Object& owner) : Tag(owner), path_properties("", *this)
{
  static constexpr double POSITION_STEP = 0.001;
  const auto category = QObject::tr("path");
  create_property<FloatProperty>(POSITION_PROPERTY_KEY)
      .set_step(POSITION_STEP)
      .set_range(0.0, 1.0)
      .set_label(QObject::tr("position"))
      .set_category(category);

  path_properties.make_properties(category);
}

PathTag::PathTag(const PathTag& other) : Tag(other), path_properties("", *this)
{
}

QString PathTag::type() const
{
  return TYPE;
}
Flag PathTag::flags() const
{
  return Tag::flags();
}

void PathTag::on_property_value_changed(Property* property)
{
  evaluate();
  Tag::on_property_value_changed(property);
}

void PathTag::evaluate()
{
  const double t = property(POSITION_PROPERTY_KEY)->value<double>();
  path_properties.apply_transformation(*owner, t, ObjectTransformation());
}

}  // namespace omm
