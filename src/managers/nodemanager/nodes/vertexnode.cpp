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
  m_position_port = &add_port<OrdinaryPort<PortType::Output>>(tr("position"));
}

QString VertexNode::output_data_type(const OutputPort& port) const
{
  Q_UNUSED(port);
  return NodeCompilerTypes::FLOATVECTOR_TYPE;
}

}  // namespace
