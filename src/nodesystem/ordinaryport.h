#pragma once

#include "nodesystem/port.h"
#include "properties/property.h"

namespace omm::nodes
{

template<PortType port_type_> class OrdinaryPort : public ConcretePortSelector<port_type_>::T
{
public:
  using BasePort = typename ConcretePortSelector<port_type_>::T;
  OrdinaryPort(Node& node, std::size_t index, const QString& label)
      : BasePort(PortFlavor::Ordinary, node, index), m_label(label)
  {
  }

  [[nodiscard]] Type data_type() const override;
  [[nodiscard]] QString label() const override
  {
    return m_label;
  }
  void set_label(const QString& label)
  {
    m_label = label;
  }

private:
  QString m_label;
};

}  // namespace omm::nodes
