#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

class MathNode : public Node
{
  Q_OBJECT
public:
  explicit MathNode(NodeModel& model);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "MathNode");
  QString type() const override
  {
    return TYPE;
  }

  static constexpr auto OPERATION_PROPERTY_KEY = "op";
  static constexpr auto A_VALUE_KEY = "a";
  static constexpr auto B_VALUE_KEY = "b";

  QString output_data_type(const OutputPort& port) const override;
  QString input_data_type(const InputPort& port) const override;
  bool accepts_input_data_type(const QString& type, const InputPort& port) const override;
  QString title() const override;
  static const Detail detail;

private:
  InputPort* m_operation_input;
  OutputPort* m_output;
};

}  // namespace omm::nodes
