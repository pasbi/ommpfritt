#pragma once

#include "properties/property.h"
#include "managers/nodemanager/port.h"

namespace omm
{

template<PortType port_type_> class OrdinaryPort : public ConcretePortSelector<port_type_>::T
{
public:
  using BasePort = typename ConcretePortSelector<port_type_>::T;
  OrdinaryPort(Node& node, std::size_t index, const QString& label, const QString& data_type)
    : BasePort(PortFlavor::Ordinary, node, index)
    , m_label(label), m_data_type(data_type)
  {
  }

  QString label() const override { return m_label; }
  QString data_type() const override { return m_data_type; }

private:
  const QString m_label;
  const QString m_data_type;

};

}  // namespace omm
