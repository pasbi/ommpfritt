#pragma once

#include "properties/property.h"
#include "managers/nodemanager/port.h"

namespace omm
{

template<PortType port_type_> class OrdinaryPort : public ConcretePortSelector<port_type_>::T
{
public:
  using BasePort = typename ConcretePortSelector<port_type_>::T;
  OrdinaryPort(Node& node, std::size_t index, const QString& label)
    : BasePort(PortFlavor::Ordinary, node, index)
    , m_label(label)
  {
  }

  explicit OrdinaryPort(const OrdinaryPort& other, Node& node)
    : ConcretePortSelector<port_type_>::T(other, node)
    , m_label(other.label()), m_data_type(other.data_type())
  {
  }

  QString label() const override { return m_label; }

  QString data_type() const override;

  std::unique_ptr<AbstractPort> clone(Node& node) const override
  {
    return std::make_unique<OrdinaryPort<port_type_>>(*this, node);
  }

private:
  const QString m_label;
  const QString m_data_type;

};

}  // namespace omm
