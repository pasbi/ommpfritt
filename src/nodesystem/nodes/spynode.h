#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

template<PortType> class OrdinaryPort;

class SpyNode : public Node
{
  Q_OBJECT
public:
  explicit SpyNode(NodeModel& model);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SpyNode");
  [[nodiscard]] QString type() const override;

  [[nodiscard]] bool accepts_input_data_type(Type type, const InputPort& port, bool with_cast) const override;
  void set_text(const QString& text);
  static const Detail detail;

private:
  OrdinaryPort<PortType::Input>* m_port;
};

}  // namespace omm::nodes
