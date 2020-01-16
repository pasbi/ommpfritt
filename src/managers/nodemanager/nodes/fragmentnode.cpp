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
  add_port<OrdinaryPort<PortType::Input>>(tr("color"));
}

QString FragmentNode::definition() const
{
  assert(language() == NodeCompiler::Language::GLSL);
  return "";
}

}  // namespace
