#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

template<PortType> class OrdinaryPort;

class Function2Node : public Node
{
  Q_OBJECT
public:
  explicit Function2Node(NodeModel& model);
  static constexpr auto OPERATION_PROPERTY_KEY = "op";
  static constexpr auto INPUT_A_PROPERTY_KEY = "a";
  static constexpr auto INPUT_B_PROPERTY_KEY = "b";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Function2Node");

  [[nodiscard]] Type output_data_type(const OutputPort& port) const override;
  [[nodiscard]] bool accepts_input_data_type(Type type, const InputPort& port, bool with_cast) const override;
  [[nodiscard]] QString type() const override;
  static const Detail detail;
  [[nodiscard]] QString title() const override;

private:
  OutputPort* m_output_port;
};

}  // namespace omm::nodes
