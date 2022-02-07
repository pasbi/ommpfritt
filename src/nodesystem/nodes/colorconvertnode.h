#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

template<PortType> class OrdinaryPort;

class ColorConvertNode : public Node
{
  Q_OBJECT
public:
  explicit ColorConvertNode(NodeModel& model);
  static constexpr auto CONVERSION_PROPERTY_KEY = "conversion";
  static constexpr auto COLOR_PROPERTY_KEY = "color";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ColorConvertNode");

  [[nodiscard]] Type output_data_type(const OutputPort& port) const override;
  [[nodiscard]] QString title() const override;
  [[nodiscard]] QString type() const override;
  static const Detail detail;

private:
  OutputPort* m_vector_output_port;
};

}  // namespace omm::nodes
