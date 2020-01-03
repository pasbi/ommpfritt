#pragma once

#include "aspects/abstractpropertyowner.h"
#include "properties/property.h"
#include "managers/nodemanager/port.h"

namespace omm
{

class Property;

template<PortType port_type_> class PropertyPort : public ConcretePortSelector<port_type_>::T
{
public:
  using BasePort = typename ConcretePortSelector<port_type_>::T;

  PropertyPort(Node& node, std::size_t index, const std::function<Property*()>& get_property)
    : BasePort(PortFlavor::Property, node, index)
    , m_get_property(get_property)
  {
  }

  explicit PropertyPort(const PropertyPort& other, Node& node)
    : BasePort(other, node)
    , m_get_property(other.m_get_property)
  {
  }

  std::unique_ptr<AbstractPort> clone(Node& node) const override
  {
    return std::make_unique<PropertyPort<port_type_>>(*this, node);
  }

  Property* property() const { return m_get_property(); }

  QString label() const override
  {
    if (Property* property = this->property(); property == nullptr) {
      return QObject::tr("undefined");
    } else {
      return property->label();
    }
  }

  QString data_type() const override;

private:
  const std::function<Property*()> m_get_property;
};

class PropertyOutputPort : public PropertyPort<PortType::Output>
{
public:
  using PropertyPort<PortType::Output>::PropertyPort;
};

class PropertyInputPort : public PropertyPort<PortType::Input>
{
public:
  using PropertyPort<PortType::Input>::PropertyPort;
  void connect(OutputPort* port) override;
};

}  // namespace omm
