#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

class LinePatternNode : public Node
{
  Q_OBJECT
public:
  explicit LinePatternNode(NodeModel& model);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "LinePatternNode");
  [[nodiscard]] QString type() const override;
  [[nodiscard]] Type output_data_type(const OutputPort& port) const override;
  [[nodiscard]] Type input_data_type(const InputPort& port) const override;
  [[nodiscard]] QString title() const override;
  static const Detail detail;

  static constexpr auto FREQUENCY_PROPERTY_KEY = "frequency";
  static constexpr auto RATIO_PROPERTY_KEY = "ratio";
  static constexpr auto LEFT_RAMP_PROPERTY_KEY = "leftramp";
  static constexpr auto RIGHT_RAMP_PROPERTY_KEY = "rightramp";

private:
  const InputPort* m_position_port = nullptr;
  const OutputPort* m_output_port = nullptr;
};

}  // namespace omm::nodes
