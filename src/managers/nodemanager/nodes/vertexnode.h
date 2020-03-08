#include "managers/nodemanager/node.h"
#include "renderers/offscreenrenderer.h"

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

  struct PortInfo {
    const OffscreenRenderer::ShaderInput input_info;
    OrdinaryPort<PortType::Output>* const port;
    bool operator==(const PortInfo& other) const { return other.port == this->port; }
    bool operator==(const AbstractPort* port) const;
    bool operator<(const PortInfo& other) const { return this->port < other.port; }
  };

  const std::set<PortInfo>& shader_inputs() const { return m_shader_inputs; }

private:
  std::set<PortInfo> m_shader_inputs;
};

}  // namespace omm
