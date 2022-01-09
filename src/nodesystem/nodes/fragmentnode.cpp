#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/ordinaryport.h"
#include "nodesystem/propertyport.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"
#include "scene/scene.h"
#include "variant.h"

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

bool FragmentNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  Q_UNUSED(port)
  assert(&port == m_input_port);
  return type == types::COLOR_TYPE;
}

}  // namespace omm::nodes
