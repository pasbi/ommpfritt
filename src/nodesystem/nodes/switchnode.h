#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

template<PortType> class OrdinaryPort;

class SwitchNode : public Node
{
  Q_OBJECT
public:
  explicit SwitchNode(NodeModel& model);
  static constexpr auto KEY_KEY = "key";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SwitchNode");

  [[nodiscard]] Type output_data_type(const OutputPort& port) const override;
  [[nodiscard]] QString title() const override;
  [[nodiscard]] bool accepts_input_data_type(Type type, const InputPort& port, bool with_cast) const override;
  [[nodiscard]] QString type() const override;
  static const Detail detail;
  [[nodiscard]] Type input_data_type(const InputPort& port) const override;
  [[nodiscard]] QString dangling_input_port_uuid(const InputPort& port) const override;

private:
  OutputPort* m_output_port;
  InputPort* m_key_input_port;
  std::vector<InputPort*> m_options;
  [[nodiscard]] InputPort* find_surrogate_for(const InputPort& port) const;
};

}  // namespace omm::nodes
