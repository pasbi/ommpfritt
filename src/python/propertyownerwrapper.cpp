#include "python/propertyownerwrapper.h"

#include "logging.h"
#include "objects/object.h"
#include "properties/floatvectorproperty.h"
#include "properties/integervectorproperty.h"
#include "properties/referenceproperty.h"
#include "properties/triggerproperty.h"
#include "python/objectwrapper.h"
#include "python/stylewrapper.h"
#include "python/tagwrapper.h"
#include "renderers/style.h"
#include "tags/tag.h"

namespace
{
template<typename WrappedT, typename WrapperT>
bool set_property_value(const py::object& value, omm::Property& property)
{
  try {
    const auto wrapper = value.cast<WrapperT>();
    auto& reference_property = dynamic_cast<omm::ReferenceProperty&>(property);
    if (reference_property.filter().kind.evaluate(WrappedT::KIND)) {
      property.set(&wrapper.wrapped);
      return true;
    } else {
      return false;
    }
  } catch (const std::exception&) {
    return false;
  }
}

}  // namespace

namespace omm::detail
{
bool set_property_value(AbstractPropertyOwner& property_owner,
                        const QString& key,
                        const py::object& value)
{
  if (property_owner.has_property(key)) {
    auto& property = *property_owner.property(key);
    if (property.type() == ReferenceProperty::TYPE()) {
      if (value.is_none()) {
        // TODO replace return-status with throw exception
        property.set(variant_type{nullptr});
        return true;
      } else if (::set_property_value<Object, ObjectWrapper>(value, property)) {
        return true;
      } else if (::set_property_value<Tag, TagWrapper>(value, property)) {
        return true;
      } else if (::set_property_value<Style, StyleWrapper>(value, property)) {
        return true;
      } else {
        LWARNING << "Attempted to set non-allowed kind of reference.";
        return false;
      }
    } else if (property.type() == TriggerProperty::TYPE()) {
      return false;
    } else if (property.type() == FloatVectorProperty::TYPE()) {
      property.set(Vec2f(value.cast<std::vector<Vec2f::element_type>>()));
      return true;
    } else if (property.type() == IntegerVectorProperty::TYPE()) {
      property.set(Vec2i(value.cast<std::vector<Vec2i::element_type>>()));
      return true;
    } else {
      property.set(value.cast<variant_type>());
      return true;
    }
  } else {
    LWARNING << "Failed to set value for key '" << key << "': key not found.";
    return false;
  }
}

}  // namespace omm::detail
