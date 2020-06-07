#include "tags/pathtag.h"
#include "properties/referenceproperty.h"
#include "properties/floatproperty.h"
#include "properties/boolproperty.h"
#include "objects/object.h"

namespace omm
{

PathTag::PathTag(Object& owner) : Tag(owner)
{
  create_property<ReferenceProperty>(PATH_REFERENCE_PROPERTY_KEY)
    .set_filter(ReferenceProperty::Filter({ Kind::Object }, {{{}}}))
    .set_label(QObject::tr("path"))
    .set_category(QObject::tr("path"));
  create_property<FloatProperty>(POSITION_PROPERTY_KEY)
    .set_step(0.001).set_range(0.0, 1.0)
    .set_label(QObject::tr("position"))
    .set_category(QObject::tr("path"));
  create_property<BoolProperty>(ALIGN_REFERENCE_PROPERTY_KEY)
    .set_label(QObject::tr("align"))
    .set_category(QObject::tr("path"));
}

QString PathTag::type() const { return TYPE; }
Flag PathTag::flags() const { return Tag::flags(); }

void PathTag::on_property_value_changed(Property* property)
{
  evaluate();
  Tag::on_property_value_changed(property);
}

void PathTag::evaluate()
{
  auto* o = property(PATH_REFERENCE_PROPERTY_KEY)->value<AbstractPropertyOwner*>();
  const double t = property(POSITION_PROPERTY_KEY)->value<double>();
  const bool align = property(ALIGN_REFERENCE_PROPERTY_KEY)->value<bool>();
  owner->set_position_on_path(o, align, t, Space::Viewport);
}

}  // namespace
