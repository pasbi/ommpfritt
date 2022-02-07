#include "nodesystem/nodes/vertexnode.h"
#include "nodesystem/ordinaryport.h"
#include "nodesystem/propertyport.h"
#include "properties/floatproperty.h"
#include "propertytypeenum.h"
#include "scene/scene.h"

namespace omm::nodes
{

const Node::Detail VertexNode::detail{
    .definitions = {{BackendLanguage::GLSL, ""}},
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "General")},
};

VertexNode::VertexNode(NodeModel& model) : Node(model)
{
  for (const auto& shader_input : OffscreenRenderer::fragment_shader_inputs) {
    auto& port = add_port<OrdinaryPort<PortType::Output>>(shader_input.tr_name());
    m_shader_inputs.insert({shader_input, &port});
  }
}

QString VertexNode::type() const
{
  return TYPE;
}

Type VertexNode::output_data_type(const OutputPort& port) const
{
  const auto it = std::find(m_shader_inputs.begin(), m_shader_inputs.end(), &port);
  assert(it != m_shader_inputs.end());
  return it->input_info.type;
}

const std::set<VertexNode::PortInfo>& VertexNode::shader_inputs() const
{
  return m_shader_inputs;
}

bool VertexNode::PortInfo::operator==(const AbstractPort* port) const
{
  return port == this->port;
}

bool VertexNode::PortInfo::operator<(const PortInfo& other) const
{
  return this->port < other.port;
}

bool VertexNode::PortInfo::operator==(const PortInfo& other) const
{
  return other.port == this->port;
}

}  // namespace omm::nodes
