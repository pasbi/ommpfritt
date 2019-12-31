#include "managers/nodemanager/nodes/spynode.h"
#include "managers/nodemanager/nodemodel.h"
#include "managers/nodemanager/ordinaryport.h"

namespace omm
{

SpyNode::SpyNode(Scene* scene)
  : Node(scene)
{
  m_port = &add_port<OrdinaryPort<PortType::Input>>(tr("value"));
}

std::unique_ptr<Node> SpyNode::clone() const
{
  return std::make_unique<SpyNode>(*this);
}

bool SpyNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  Q_UNUSED(port)
  return type != NodeCompilerTypes::INVALID_TYPE;
}

void SpyNode::set_text(const QString& text)
{
  m_port->set_label(text);
  model()->notify_node_shape_changed();
}

}  // namespace
