#pragma once

#include "nodesystem/node.h"
#include "nodesystem/ordinaryport.h"

namespace omm::nodes
{

class CastNode : public Node
{
  Q_OBJECT
public:
  explicit CastNode(NodeModel& model, const QString& type, Type target_type);
  [[nodiscard]] Type output_data_type(const OutputPort& port) const override;
  [[nodiscard]] bool accepts_input_data_type(Type type, const InputPort& port) const override;
  [[nodiscard]] QString function_name(const std::size_t i) const override;
  static const Detail detail;
  [[nodiscard]] QString title() const override;
  [[nodiscard]] QString type() const override;

private:
  const QString m_type;
  const Type m_target_type;
  static bool is_convertible(Type from, Type to);
  OutputPort* m_output_port;
  InputPort* m_input_port;
};

}  // namespace omm::nodes
