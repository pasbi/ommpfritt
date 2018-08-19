#pragma once

#include <glog/logging.h>
#include <assert.h>
#include <map>
#include <memory>
#include <typeinfo>
#include "property.h"

#define DEFINE_CLASSNAME(CLASSNAME) \
  std::string class_name() const override { return CLASSNAME; }

class HasProperties
{
public:
  using PropertyKey = std::string;
  using PropertyMap = std::map<PropertyKey, std::unique_ptr<Property>>;

  virtual ~HasProperties();

  Property& property(const PropertyKey& key) const
  {
    return *m_properties.at(key);
  }

  std::vector<PropertyKey> property_keys() const;

protected:
  void add_property(const PropertyKey& key, std::unique_ptr<Property> property);
  virtual std::string class_name() const = 0;

private:
  PropertyMap m_properties;
  
};
