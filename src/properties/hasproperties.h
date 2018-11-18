#pragma once

#include <glog/logging.h>
#include <assert.h>
#include <map>
#include <memory>
#include <typeinfo>
#include "properties/property.h"
#include "external/json_fwd.hpp"
#include "properties/propertymap.h"
#include "serializers/serializable.h"
#include "properties/typedproperty.h"

namespace omm
{

class HasProperties : public Serializable, public AbstractPropertyObserver
{
public:
  explicit HasProperties();
  HasProperties(HasProperties&& other);
  virtual ~HasProperties();
  using Key = PropertyMap::key_type;
  Property& property(const Key& key) const;

  template<typename ValueT> TypedProperty<ValueT>& property(const Key& key) const
  {
    assert(has_property<ValueT>(key));
    return *this->property(key).cast<ValueT>();
  }

  bool has_property(const Key& key) const;

  template<typename ValueT> bool has_property(const Key& key) const
  {
    return this->has_property(key) && this->property(key).is_type<ValueT>();
  }

  const PropertyMap& properties() const;

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  virtual std::string name() const = 0;
  virtual void on_property_value_changed() override;

protected:
  Property& add_property(const Key& key, std::unique_ptr<Property> property);

private:
  PropertyMap m_properties;
};

}  // namespace omm
