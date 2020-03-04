#include "managers/nodemanager/nodes/vertexnode.h"
#include "managers/nodemanager/ordinaryport.h"
#include "properties/floatproperty.h"
#include "managers/nodemanager/propertyport.h"
#include "variant.h"
#include "scene/scene.h"
#include "properties/optionsproperty.h"

namespace omm
{

const Node::Detail VertexNode::detail { {
    { AbstractNodeCompiler::Language::GLSL, "" } } };

VertexNode::VertexNode(NodeModel& model)
  : Node(model)
{
  for (const auto& varying : OffscreenRenderer::varyings) {
    auto& port = add_port<OrdinaryPort<PortType::Output>>(varying.tr_name());
    m_varying_ports.insert({ varying, &port });
  }
}

QString VertexNode::output_data_type(const OutputPort& port) const
{
  const auto it = std::find(m_varying_ports.begin(), m_varying_ports.end(), &port);
  assert(it != m_varying_ports.end());
  return it->varying_info.type;
}

bool VertexNode::PortInfo::operator==(const AbstractPort* port) const
{
  return port == this->port;
}

}  // namespace
