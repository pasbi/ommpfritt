#pragma once

#include <memory>
#include <QListWidgetItem>
#include "properties/property.h"

namespace omm
{

class Property;
class PropertyItem : public QListWidgetItem
{
public:
  explicit PropertyItem();
  template<typename PropertyT> void set_property_type()
  {
    m_property = std::make_unique<PropertyT>();
  }

  template<typename PropertyT> PropertyT& property() const
  {
    return static_cast<PropertyT&>(*m_property);
  }

  Property& property() const
  {
    return *m_property;
  }

  std::string property_type() const;
  QVariant data(int role) const;

private:
  std::unique_ptr<Property> m_property;
};

}  // namespace omm