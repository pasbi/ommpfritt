#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

template<PortType> class OrdinaryPort;

class FragmentNode : public Node
{
  Q_OBJECT
public:
  explicit FragmentNode(NodeModel& model);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "FragmentNode");
  [[nodiscard]] QString type() const override;
  [[nodiscard]] bool accepts_input_data_type(Type type, const InputPort& port, bool with_cast) const override;

  static constexpr auto COLOR_PROPERTY_KEY = "color";

  static const Detail detail;
  [[nodiscard]] OrdinaryPort<PortType::Input>& input_port() const;
  [[nodiscard]] bool copyable() const override;

private:
  OrdinaryPort<PortType::Input>* m_input_port = nullptr;
};

}  // namespace omm::nodes
