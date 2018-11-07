#pragma once

#include <glog/logging.h>
#include <assert.h>
#include <map>
#include <memory>
#include <typeinfo>
#include "property.h"
#include "external/json_fwd.hpp"
#include "properties/propertymap.h"

#define DEFINE_CLASSNAME(CLASSNAME) \
  std::string class_name() const override { return CLASSNAME; }

namespace omm
{

class HasProperties
{
public:
  virtual ~HasProperties();
  using Key = PropertyMap::key_type;
  Property& property(const Key& key) const;
  template<typename ValueT> TypedProperty<ValueT>& property(const Key& key) const
  {
    return property(key).cast<ValueT>();
  }
  const PropertyMap& properties() const;
  Property& add_property(const Key& key, std::unique_ptr<Property> property);

  virtual std::string class_name() const = 0;

private:
  PropertyMap m_properties;
};

}  // namespace omm
