#include "managers/nodemanager/node.h"

namespace omm
{

template<PortType> class OrdinaryPort;

class VertexNode : public Node
{
  Q_OBJECT
public:
  explicit VertexNode(NodeModel& model);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "VertexNode");
  QString type() const override { return TYPE; }
  QString output_data_type(const OutputPort& port) const override;

  static const Detail detail;
  OrdinaryPort<PortType::Output>& position_port() const { return *m_position_port; }

private:
  OrdinaryPort<PortType::Output>* m_position_port = nullptr;
};

}  // namespace omm
