#pragma once

#include "nodesystem/port.h"
#include "properties/property.h"
#include <QObject>

namespace omm
{
class Property;
}  // namespace omm

namespace omm::nodes
{

template<PortType port_type_> class PropertyPort : public ConcretePortSelector<port_type_>::T
{
public:
  using BasePort = typename ConcretePortSelector<port_type_>::T;

  PropertyPort(Node& node, std::size_t index, const std::function<Property*()>& get_property)
      : BasePort(PortFlavor::Property, node, index), m_get_property(get_property)
  {
  }

  explicit PropertyPort(const PropertyPort& other, Node& node)
      : BasePort(other, node), m_get_property(other.m_get_property)
  {
  }

  [[nodiscard]] Property* property() const
  {
    return m_get_property();
  }

  [[nodiscard]] QString label() const override
  {
    if (Property* property = this->property(); property == nullptr) {
      return QObject::tr("undefined");
    } else {
      return property->label();
    }
  }

  [[nodiscard]] Type data_type() const override;

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

}  // namespace omm::nodes
