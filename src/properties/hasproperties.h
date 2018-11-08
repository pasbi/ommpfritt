#pragma once

#include <glog/logging.h>
#include <assert.h>
#include <map>
#include <memory>
#include <typeinfo>
#include "property.h"
#include "external/json_fwd.hpp"
#include "properties/propertymap.h"
#include "serializers/serializable.h"

namespace omm
{

class HasProperties : public Serializable
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

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

protected:
  Property& add_property(const Key& key, std::unique_ptr<Property> property);

private:
  PropertyMap m_properties;
};

}  // namespace omm
