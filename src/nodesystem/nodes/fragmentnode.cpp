#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/ordinaryport.h"
#include "nodesystem/propertyport.h"
#include "properties/floatproperty.h"
#include "propertytypeenum.h"
#include "scene/scene.h"

namespace omm::nodes
{

const Node::Detail FragmentNode::detail {
  .definitions = {{BackendLanguage::GLSL, ""}},
  .menu_path = {}
};

FragmentNode::FragmentNode(NodeModel& model) : Node(model)
{
  m_input_port = &add_port<OrdinaryPort<PortType::Input>>(tr("color"));
}

QString FragmentNode::type() const
{
  return TYPE;
}

bool FragmentNode::accepts_input_data_type(const Type type, const InputPort& port, bool with_cast) const
{
  Q_UNUSED(with_cast)
  Q_UNUSED(port)
  assert(&port == m_input_port);
  return type == Type::Color;
}

OrdinaryPort<PortType::Input>& FragmentNode::input_port() const
{
  return *m_input_port;
}

bool FragmentNode::copyable() const
{
  return false;
}

}  // namespace omm::nodes
