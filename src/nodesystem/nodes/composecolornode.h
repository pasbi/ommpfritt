#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

template<PortType> class OrdinaryPort;

class ComposeColorNode : public Node
{
  Q_OBJECT
public:
  explicit ComposeColorNode(NodeModel& model);
  static constexpr auto INPUT_R_PROPERTY_KEY = "r";
  static constexpr auto INPUT_G_PROPERTY_KEY = "g";
  static constexpr auto INPUT_B_PROPERTY_KEY = "b";
  static constexpr auto INPUT_A_PROPERTY_KEY = "a";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ComposeColorNode");

  [[nodiscard]] Type output_data_type(const OutputPort& port) const override;
  [[nodiscard]] QString title() const override;
  [[nodiscard]] bool accepts_input_data_type(Type type, const InputPort& port, bool with_cast) const override;
  [[nodiscard]] QString type() const override;
  static const Detail detail;

private:
  OutputPort* m_output_port;
};

}  // namespace omm::nodes
