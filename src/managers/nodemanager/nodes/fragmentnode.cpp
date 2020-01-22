#include "managers/nodemanager/nodes/fragmentnode.h"
#include "managers/nodemanager/ordinaryport.h"
#include "properties/floatproperty.h"
#include "managers/nodemanager/propertyport.h"
#include "variant.h"
#include "scene/scene.h"
#include "properties/optionsproperty.h"

namespace omm
{

const Node::Detail FragmentNode::detail { {
    { AbstractNodeCompiler::Language::GLSL, "" } } };

FragmentNode::FragmentNode(NodeModel& model)
  : Node(model)
{
  const QString category = tr("Node");
  m_input_port = &add_port<OrdinaryPort<PortType::Input>>(tr("color"));
}

bool FragmentNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  assert(&port == m_input_port);
  return type == NodeCompilerTypes::COLOR_TYPE;
}

QString FragmentNode::port_name() const
{
  return m_input_port->uuid();
}

}  // namespace
