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

namespace omm
{

class HasProperties
{
public:
  using Key = std::string;
  class PropertyMap : public std::map<Key, std::unique_ptr<Property>>
  {
  public:
    nlohmann::json to_json() const;
  };

  virtual ~HasProperties();

  Property& property(const Key& key) const;
  std::vector<Key> property_keys() const;
  nlohmann::json to_json() const;

protected:
  void add_property(const Key& key, std::unique_ptr<Property> property);
  virtual std::string class_name() const = 0;

private:
  PropertyMap m_properties;
  std::vector<Key> m_property_keys;

};

}  // namespace omm
