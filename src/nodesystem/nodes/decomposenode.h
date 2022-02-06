#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

template<PortType> class OrdinaryPort;

class DecomposeNode : public Node
{
  Q_OBJECT
public:
  explicit DecomposeNode(NodeModel& model);
  static constexpr auto INPUT_PROPERTY_KEY = "in";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "DecomposeNode");

  [[nodiscard]] Type output_data_type(const OutputPort& port) const override;
  [[nodiscard]] QString title() const override;
  [[nodiscard]] bool accepts_input_data_type(Type type, const InputPort& port, bool with_cast) const override;
  [[nodiscard]] QString type() const override;
  static const Detail detail;

private:
  OutputPort* m_output_x_port;
  OutputPort* m_output_y_port;
};

}  // namespace omm::nodes
