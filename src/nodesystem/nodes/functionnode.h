#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
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

  [[nodiscard]] Type output_data_type(const OutputPort&) const override;
  [[nodiscard]] bool accepts_input_data_type(Type type, const InputPort&, bool with_cast) const override;
  [[nodiscard]] QString type() const override;
  static const Detail detail;
  [[nodiscard]] QString title() const override;

private:
  OutputPort* m_output_port;
};

}  // namespace omm::nodes
