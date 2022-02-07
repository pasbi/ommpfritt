#pragma once

#include "nodesystem/node.h"
#include "renderers/offscreenrenderer.h"

namespace omm::nodes
{

template<PortType> class OrdinaryPort;

class VertexNode : public Node
{
  Q_OBJECT
public:
  explicit VertexNode(NodeModel& model);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "VertexNode");
  [[nodiscard]] QString type() const override;
  [[nodiscard]] Type output_data_type(const OutputPort& port) const override;
  static const Detail detail;

  struct PortInfo {
    const OffscreenRenderer::ShaderInput input_info;
    OrdinaryPort<PortType::Output>* const port;
    bool operator==(const PortInfo& other) const;
    bool operator==(const AbstractPort* port) const;
    bool operator<(const PortInfo& other) const;
  };

  [[nodiscard]] const std::set<PortInfo>& shader_inputs() const;

private:
  std::set<PortInfo> m_shader_inputs;
};

}  // namespace omm::nodes
