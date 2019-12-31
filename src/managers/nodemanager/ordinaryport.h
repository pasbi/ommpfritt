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
    , m_label(other.label())
  {
  }

  std::unique_ptr<AbstractPort> clone(Node& node) const override
  {
    return std::make_unique<OrdinaryPort<port_type_>>(*this, node);
  }

  QString data_type() const override;
  QString label() const override { return m_label; }
  void set_label(const QString& label) { m_label = label; }

private:
  QString m_label;
};

}  // namespace omm
