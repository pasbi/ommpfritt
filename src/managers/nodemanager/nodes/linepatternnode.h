#pragma once

#include "managers/nodemanager/node.h"

namespace omm
{

class LinePatternNode : public Node
{
public:
  explicit LinePatternNode(NodeModel& model);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "LinePatternNode");
  QString type() const override { return TYPE; }
  QString output_data_type(const OutputPort& port) const override;
  QString input_data_type(const InputPort& port) const override;
  QString title() const override;
  static const Detail detail;

  static constexpr auto FREQUENCY_PROPERTY_KEY = "frequency";
  static constexpr auto RATIO_PROPERTY_KEY = "ratio";
  static constexpr auto LEFT_RAMP_PROPERTY_KEY = "leftramp";
  static constexpr auto RIGHT_RAMP_PROPERTY_KEY = "rightramp";

private:
  const InputPort* m_position_port = nullptr;
  const OutputPort* m_output_port = nullptr;
};

}  // namespace omm
