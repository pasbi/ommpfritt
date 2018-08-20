#pragma once

#include <glog/logging.h>
#include <assert.h>
#include <map>
#include <memory>
#include <typeinfo>
#include "property.h"
#include "external/json_fwd.hpp"

#define DEFINE_CLASSNAME(CLASSNAME) \
  std::string class_name() const override { return CLASSNAME; }

class HasProperties
{
public:
  using PropertyKey = std::string;
  class PropertyMap : public std::map<PropertyKey, std::unique_ptr<Property>>
  {
  public:
    nlohmann::json to_json() const;
  };

  virtual ~HasProperties();

  Property& property(const PropertyKey& key) const
  {
    return *m_properties.at(key);
  }

  std::vector<PropertyKey> property_keys() const;
  const PropertyMap& property_map() const;

protected:
  void add_property(const PropertyKey& key, std::unique_ptr<Property> property);
  virtual std::string class_name() const = 0;

private:
  PropertyMap m_properties;
  
};

