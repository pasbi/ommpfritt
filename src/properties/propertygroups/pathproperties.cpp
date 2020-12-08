#include "properties/propertygroups/pathproperties.h"
#include "objects/object.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/integerproperty.h"
#include "properties/optionproperty.h"
#include "properties/referenceproperty.h"

namespace omm
{
const std::set<QString> PathProperties::keys{PATH_REFERENCE_PROPERTY_KEY,
                                             ALIGN_PROPERTY_KEY,
                                             SEGMENT_PROPERTY_KEY,
                                             INTERPOLATION_KEY};

void PathProperties::make_properties(const QString& category) const
{
  create_property<ReferenceProperty>(PATH_REFERENCE_PROPERTY_KEY)
      .set_filter(PropertyFilter({Kind::Object}, {{{}}}))
      .set_label(QObject::tr("path"))
      .set_category(category);
  create_property<IntegerProperty>(SEGMENT_PROPERTY_KEY)
      .set_special_value("All Segments")
      .set_range(-1, std::numeric_limits<int>::max())
      .set_label(QObject::tr("segment"))
      .set_category(category);
  create_property<OptionProperty>(INTERPOLATION_KEY)
      .set_options({QObject::tr("Natural"), QObject::tr("Distance")})
      .set_label(QObject::tr("Interpolation"))
      .set_category(category);
  create_property<BoolProperty>(ALIGN_PROPERTY_KEY)
      .set_label(QObject::tr("align"))
      .set_category(category);
}

void PathProperties::apply_transformation(Object& object,
                                          double t,
                                          const ObjectTransformation& transformation) const
{
  const auto* path_object = property_value<AbstractPropertyOwner*>(PATH_REFERENCE_PROPERTY_KEY);
  if (const auto* const path = kind_cast<const Object*>(path_object); path != nullptr) {
    const auto interpolation = property_value<Object::Interpolation>(INTERPOLATION_KEY);
    const int path_index = property_value<int>(SEGMENT_PROPERTY_KEY);
    const auto time = path->compute_path_vector_time(path_index, t, interpolation);
    const auto gt = path->global_transformation(Space::Scene);
    const auto location = transformation.apply(gt.apply(path->pos(time)));
    object.set_oriented_position(location, property_value<bool>(ALIGN_PROPERTY_KEY));
  }
}

}  // namespace omm
