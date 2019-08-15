#pragma once

#include <string>
#include <QFont>
#include <QTextOption>
#include "aspects/propertyowner.h"

namespace omm
{

class PropertyGroup
{
public:
  explicit PropertyGroup(const std::string& prefix, AbstractPropertyOwner& property_owner);
  PropertyGroup(const PropertyGroup& other) = delete;
  virtual void make_properties(const std::string& group) const = 0;
  virtual ~PropertyGroup() = default;

private:
  const std::string m_prefix;
  AbstractPropertyOwner& m_property_owner;

protected:
  std::string key(const std::string& key) const;
  template<typename T> T property_value(const std::string& key) const
  {
    return m_property_owner.property(this->key(key))->value<T>();
  }

  template<typename PropertyT, typename... Args>
  decltype(auto) create_property(const std::string& key, Args&&... args) const
  {
    return m_property_owner.create_property<PropertyT>(this->key(key), std::forward<Args>(args)...);
  }
};

}  // namespace omm
