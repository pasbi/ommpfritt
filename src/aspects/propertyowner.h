#pragma once

#include <glog/logging.h>
#include <cassert>
#include <map>
#include <memory>
#include <typeinfo>

#include "properties/property.h"
#include "orderedmap.h"
#include "external/json_fwd.hpp"
#include "aspects/serializable.h"
#include "properties/typedproperty.h"
#include "properties/stringproperty.h"
#include "common.h"


namespace omm
{

using PropertyMap = OrderedMap<std::string, std::unique_ptr<Property>>;
using SetOfPropertyOwner = std::set<omm::AbstractPropertyOwner*>;

class AbstractPropertyOwner : public virtual Serializable, public AbstractPropertyObserver
{
public:
  enum class Kind { None = 0x0,
                    Tag = 0x1, Style = 0x2, Object = 0x4,
                    All = Tag | Style | Object };
  explicit AbstractPropertyOwner();
  AbstractPropertyOwner(AbstractPropertyOwner&& other);
  virtual ~AbstractPropertyOwner();
  using Key = PropertyMap::key_type;
  Property& property(const Key& key) const;

  template<typename PropertyT> PropertyT& property(const Key& key) const
  {
    assert(has_property<PropertyT>(key));
    return *this->property(key).cast<PropertyT>();
  }

  bool has_property(const Key& key) const;

  template<typename PropertyT> bool has_property(const Key& key) const
  {
    return this->has_property(key) && this->property(key).is_type<PropertyT>();
  }

  const PropertyMap& properties() const;

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  virtual std::string name() const;
  virtual void on_property_value_changed(Property& property) override;
  virtual Kind kind() const = 0;

  template<typename T> T* cast()
  {
    if (kind() == T::KIND) {
      return static_cast<T*>(this);
    } else {
      return nullptr;
    }
  }

  template<typename T, typename AbstractPropertyOwnerT, template<typename...> class ContainerT>
  static ContainerT<T*> cast(const ContainerT<AbstractPropertyOwnerT*>& ss)
  {
    const auto f = [](AbstractPropertyOwnerT* a) -> T* {
      if (a->kind() == T::KIND) {
        return static_cast<T*>(a);
      } else {
        return nullptr;
      }
    };
    return ::filter_if(::transform<T*>(ss, f), ::is_not_null);
  }

  static const std::string NAME_PROPERTY_KEY;

protected:
  Property& add_property(const Key& key, std::unique_ptr<Property> property);

private:
  PropertyMap m_properties;
};

template<AbstractPropertyOwner::Kind kind_> class PropertyOwner : public AbstractPropertyOwner
{
public:
  using AbstractPropertyOwner::AbstractPropertyOwner;
  static constexpr Kind KIND = kind_;
  Kind kind() const override { return KIND; }
};

}  // namespace omm

omm::AbstractPropertyOwner::Kind operator|( omm::AbstractPropertyOwner::Kind a,
                                            omm::AbstractPropertyOwner::Kind b );
omm::AbstractPropertyOwner::Kind operator&( omm::AbstractPropertyOwner::Kind a,
                                            omm::AbstractPropertyOwner::Kind b );
omm::AbstractPropertyOwner::Kind operator~( omm::AbstractPropertyOwner::Kind a );
bool operator!(omm::AbstractPropertyOwner::Kind a);
