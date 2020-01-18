#include "managers/nodemanager/nodes/fragmentnode.h"
#include "managers/nodemanager/ordinaryport.h"
#include "properties/floatproperty.h"
#include "managers/nodemanager/propertyport.h"
#include "variant.h"
#include "scene/scene.h"
#include "properties/optionsproperty.h"

namespace omm
{

const Node::Detail FragmentNode::detail { { NodeCompiler::Language::GLSL } };

FragmentNode::FragmentNode(Scene* scene)
  : Node(scene)
{
  const QString category = tr("Node");
  m_input_port = &add_port<OrdinaryPort<PortType::Input>>(tr("color"));
}

bool FragmentNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  assert(&port == m_input_port);
  return type == NodeCompilerTypes::COLOR_TYPE;
}

QString FragmentNode::definition() const
{
  assert(language() == NodeCompiler::Language::GLSL);
  return "";
}

}  // namespace
