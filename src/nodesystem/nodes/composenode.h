#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

template<PortType> class OrdinaryPort;

class ComposeNode : public Node
{
  Q_OBJECT
public:
  explicit ComposeNode(NodeModel& model);
  static constexpr auto INPUT_X_PROPERTY_KEY = "x";
  static constexpr auto INPUT_Y_PROPERTY_KEY = "y";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ComposeNode");

  [[nodiscard]] Type output_data_type(const OutputPort& port) const override;
  [[nodiscard]] QString title() const override;
  [[nodiscard]] bool accepts_input_data_type(Type type, const InputPort& port, bool with_cast) const override;
  [[nodiscard]] QString type() const override;
  static const Detail detail;

private:
  OutputPort* m_output_port;
};

}  // namespace omm::nodes
