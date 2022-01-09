#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

template<PortType> class OrdinaryPort;

class FragmentNode : public Node
{
  Q_OBJECT
public:
  explicit FragmentNode(NodeModel& model);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "FragmentNode");
  QString type() const override
  {
    return TYPE;
  }
  bool accepts_input_data_type(const QString& type, const InputPort& port) const override;

  static constexpr auto COLOR_PROPERTY_KEY = "color";

  static const Detail detail;
  OrdinaryPort<PortType::Input>& input_port() const
  {
    return *m_input_port;
  }
  bool copyable() const override
  {
    return false;
  }

private:
  OrdinaryPort<PortType::Input>* m_input_port = nullptr;
};

}  // namespace omm::nodes
