#pragma once

#include "nodesystem/node.h"

namespace omm
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

  QString output_data_type(const OutputPort& port) const override;
  bool accepts_input_data_type(const QString& type, const InputPort& port) const override;
  QString type() const override
  {
    return TYPE;
  }
  static const Detail detail;
  QString title() const override;

private:
  OutputPort* m_output_port;
};

}  // namespace omm
