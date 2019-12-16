#pragma once

#include "properties/property.h"
#include "managers/nodemanager/port.h"

namespace omm
{

class Property;

template<PortType port_type_> class PropertyPort : public ConcretePortSelector<port_type_>::T
{
public:
  using BasePort = typename ConcretePortSelector<port_type_>::T;
  PropertyPort(Node& node, std::size_t index, Property& property)
    : BasePort(PortFlavor::Property, node, index, property.label()), property(property)
  {
  }

  Property& property;
};

}  // namespace omm
