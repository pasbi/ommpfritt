#include "python/propertyownerwrapper.h"

#include "python/objectwrapper.h"
#include "python/tagwrapper.h"
#include "python/stylewrapper.h"
#include "properties/referenceproperty.h"
#include "properties/triggerproperty.h"
#include "renderers/style.h"
#include "objects/object.h"
#include "properties/vectorproperty.h"
#include "tags/tag.h"

namespace
{

template<typename WrappedT, typename WrapperT>
bool set_property_value(const py::object& value, omm::Property& property)
{
  try {
    const auto wrapper = value.cast<WrapperT>();
    auto& reference_property = static_cast<omm::ReferenceProperty&>(property);
    if (!!(reference_property.allowed_kinds() & WrappedT::KIND)) {
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

bool set_property_value( AbstractPropertyOwner& property_owner,
                         const std::string& key, const py::object& value )
{
  if (property_owner.has_property(key)) {
    auto& property = property_owner.property(key);
    if (property.type() == ReferenceProperty::TYPE) {
      if (value.is_none()) {
        // TODO replace return-status with throw exception
        property.set(nullptr);
        return true;
      } else if (::set_property_value<Object, ObjectWrapper>(value, property)) {
        return true;
      } else if (::set_property_value<Tag, TagWrapper>(value, property)) {
        return true;
      } else if (::set_property_value<Style, StyleWrapper>(value, property)) {
        return true;
      } else {
        LOG(WARNING) << "Attempted to set non-allowed kind of reference.";
        return false;
      }
    } else if (property.type() == TriggerProperty::TYPE) {
      return false;
    } else if (property.type() == FloatVectorProperty::TYPE) {
      property.set(Vec2f(value.cast<std::vector<Vec2f::element_type>>()));
      return true;
    } else if (property.type() == IntegerVectorProperty::TYPE) {
      property.set(Vec2i(value.cast<std::vector<Vec2i::element_type>>()));
      return true;
    } else {
      property.set(value.cast<Property::variant_type>());
      return true;
    }
  } else {
    return false;
  }
}

}  // namespace omm
