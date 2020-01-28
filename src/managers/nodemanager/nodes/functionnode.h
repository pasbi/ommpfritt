#pragma once

#include "managers/nodemanager/node.h"

namespace omm
{

template<PortType> class OrdinaryPort;

class FunctionNode : public Node
{
  Q_OBJECT
public:
  explicit FunctionNode(NodeModel& model);
  static constexpr auto OPERATION_PROPERTY_KEY = "op";
  static constexpr auto INPUT_A_PROPERTY_KEY = "a";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "FunctionNode");

  QString output_data_type(const OutputPort& port) const override;
  bool accepts_input_data_type(const QString& type, const InputPort& port) const override;
  QString type() const override { return TYPE; }
  static const Detail detail;

private:
  OutputPort* m_output_port;
};

}  // namespace omm
