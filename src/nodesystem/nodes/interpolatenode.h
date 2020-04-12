#pragma once

#include "nodesystem/node.h"

namespace omm
{

class InterpolateNode : public Node
{
  Q_OBJECT
public:
  explicit InterpolateNode(NodeModel& model);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "InterpolateNode");
  QString type() const override { return TYPE; }
  QString output_data_type(const OutputPort& port) const override;
  QString input_data_type(const InputPort& port) const override;
  bool accepts_input_data_type(const QString& type, const InputPort& port) const override;
  static const Detail detail;

  static constexpr auto RAMP_PROPERTY_KEY = "ramp";
  static constexpr auto LEFT_VALUE_KEY = "left";
  static constexpr auto RIGHT_VALUE_KEY = "right";
  static constexpr auto BALANCE_PROPERTY_KEY = "balance";

private:
  OutputPort* m_output;

};

}  // namespace omm
