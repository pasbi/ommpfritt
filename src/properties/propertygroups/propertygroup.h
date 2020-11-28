#pragma once

#include "aspects/propertyowner.h"
#include <QFont>
#include <QTextOption>
#include <string>

namespace omm
{
class PropertyGroup
{
public:
  explicit PropertyGroup(const QString& prefix, AbstractPropertyOwner& property_owner);
  virtual ~PropertyGroup() = default;
  PropertyGroup(const PropertyGroup&) = delete;
  PropertyGroup(PropertyGroup&&) = delete;
  PropertyGroup& operator=(const PropertyGroup&) = delete;
  PropertyGroup& operator=(PropertyGroup&&) = delete;

  virtual void make_properties(const QString& group) const = 0;

private:
  const QString m_prefix;
  AbstractPropertyOwner& m_property_owner;
  mutable std::set<QString> m_keys;

protected:
  template<typename T> T property_value(const QString& key) const
  {
    return m_property_owner.property(m_prefix + key)->value<T>();
  }

  template<typename PropertyT, typename... Args>
  decltype(auto) create_property(const QString& key, Args&&... args) const
  {
    const auto pkey = m_prefix + key;
    m_keys.insert(pkey);
    return m_property_owner.create_property<PropertyT>(pkey, std::forward<Args>(args)...);
  }
};

}  // namespace omm
