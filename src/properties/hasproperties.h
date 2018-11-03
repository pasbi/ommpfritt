#pragma once

#include <glog/logging.h>
#include <assert.h>
#include <map>
#include <memory>
#include <typeinfo>
#include "property.h"
#include "external/json_fwd.hpp"
#include "orderedmap.h"

#define DEFINE_CLASSNAME(CLASSNAME) \
  std::string class_name() const override { return CLASSNAME; }

namespace omm
{

class HasProperties
{
public:
  using Key = std::string;
  using PropertyMap = OrderedMap<Key, std::unique_ptr<Property>>;
  virtual ~HasProperties();

  Property& property(const Key& key) const;
  template<typename ValueT> TypedProperty<ValueT>& property(const Key& key) const
  {
    return property(key).cast<ValueT>();
  }
  const std::vector<Key>& property_keys() const;
  nlohmann::json to_json() const;

protected:
  void add_property(const Key& key, std::unique_ptr<Property> property);

  template<typename PropertyT, typename... Args> void add_property(const Key& key, Args&&... args)
  {
    add_property(key, std::make_unique<PropertyT>(std::forward<Args>(args)...));
  }

  virtual std::string class_name() const = 0;

private:
  PropertyMap m_properties;
};

}  // namespace omm
