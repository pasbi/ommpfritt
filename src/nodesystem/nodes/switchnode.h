#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

template<PortType> class OrdinaryPort;

class SwitchNode : public Node
{
  Q_OBJECT
public:
  explicit SwitchNode(NodeModel& model);
  static constexpr auto KEY_KEY = "key";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SwitchNode");

  QString output_data_type(const OutputPort& port) const override;
  QString title() const override;
  bool accepts_input_data_type(const QString& type, const InputPort& port) const override;
  QString type() const override;
  static const Detail detail;

private:
  OutputPort* m_output_port;
  std::vector<InputPort*> m_options;
};

}  // namespace omm::nodes
