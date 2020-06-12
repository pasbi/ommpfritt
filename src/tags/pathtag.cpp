#include "tags/pathtag.h"
#include "properties/optionproperty.h"
#include "properties/referenceproperty.h"
#include "properties/integerproperty.h"
#include "properties/floatproperty.h"
#include "properties/boolproperty.h"
#include "objects/object.h"

namespace omm
{

PathTag::PathTag(Object& owner) : Tag(owner)
{
  const auto category = QObject::tr("path");
  create_property<ReferenceProperty>(PATH_REFERENCE_PROPERTY_KEY)
    .set_filter(ReferenceProperty::Filter({ Kind::Object }, {{{}}}))
    .set_label(QObject::tr("path"))
    .set_category(category);
  create_property<IntegerProperty>(SEGMENT_PROPERTY_KEY)
    .set_special_value("All Segments")
    .set_range(-1, std::numeric_limits<int>::max())
    .set_label(QObject::tr("segment"))
    .set_category(category);
  create_property<FloatProperty>(POSITION_PROPERTY_KEY)
    .set_step(0.001).set_range(0.0, 1.0)
    .set_label(QObject::tr("position"))
    .set_category(category);
  create_property<OptionProperty>(INTERPOLATION_KEY)
    .set_options({QObject::tr("Natural"), QObject::tr("Distance")})
    .set_label(QObject::tr("Interpolation"))
    .set_category(category);
  create_property<BoolProperty>(ALIGN_REFERENCE_PROPERTY_KEY)
    .set_label(QObject::tr("align"))
    .set_category(category);
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
  const auto interpolation = property(INTERPOLATION_KEY)->value<Object::Interpolation>();

  if (const auto object = kind_cast<Object*>(o); object != nullptr) {
    const auto path_index = property(SEGMENT_PROPERTY_KEY)->value<int>();
    const auto time = path_index == -1
                    ? object->compute_path_vector_time(t, interpolation)
                    : object->compute_path_vector_time(path_index, t, interpolation);
    owner->set_position_on_path(*object, align, time);
  }
}

}  // namespace
