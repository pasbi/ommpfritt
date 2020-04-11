#include "nodesystem/nodes/vertexnode.h"
#include "nodesystem/ordinaryport.h"
#include "properties/floatproperty.h"
#include "nodesystem/propertyport.h"
#include "variant.h"
#include "scene/scene.h"

namespace omm
{

const Node::Detail VertexNode::detail {
  {
    { AbstractNodeCompiler::Language::GLSL, "" }
  },
  {
    QT_TRANSLATE_NOOP("NodeMenuPath", "General"),
  },
};

VertexNode::VertexNode(NodeModel& model)
  : Node(model)
{
  for (const auto& shader_input : OffscreenRenderer::fragment_shader_inputs) {
    auto& port = add_port<OrdinaryPort<PortType::Output>>(shader_input.tr_name());
    m_shader_inputs.insert({ shader_input, &port });
  }
}

QString VertexNode::output_data_type(const OutputPort& port) const
{
  const auto it = std::find(m_shader_inputs.begin(), m_shader_inputs.end(), &port);
  assert(it != m_shader_inputs.end());
  return it->input_info.type;
}

bool VertexNode::PortInfo::operator==(const AbstractPort* port) const
{
  return port == this->port;
}

}  // namespace
