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
  explicit PropertyGroup(const QString& prefix, AbstractPropertyOwner& property_owner);
  PropertyGroup(const PropertyGroup& other) = delete;
  virtual void make_properties(const QString& group) const = 0;
  virtual ~PropertyGroup() = default;

private:
  const QString m_prefix;
  AbstractPropertyOwner& m_property_owner;

protected:
  QString key(const QString& key) const;
  template<typename T> T property_value(const QString& key) const
  {
    return m_property_owner.property(this->key(key))->value<T>();
  }

  template<typename PropertyT, typename... Args>
  decltype(auto) create_property(const QString& key, Args&&... args) const
  {
    return m_property_owner.create_property<PropertyT>(this->key(key), std::forward<Args>(args)...);
  }
};

}  // namespace omm
