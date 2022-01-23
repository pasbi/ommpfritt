#include "nodesystem/nodes/spynode.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/ordinaryport.h"

namespace omm::nodes
{

const Node::Detail SpyNode::detail{
    .definitions = {{BackendLanguage::Python, ""}},
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "General")},
};

SpyNode::SpyNode(NodeModel& model) : Node(model)
{
  m_port = &add_port<OrdinaryPort<PortType::Input>>(tr("value"));
}

QString SpyNode::type() const
{
  return TYPE;
}

bool SpyNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  Q_UNUSED(port)
  return type != types::INVALID_TYPE;
}

void SpyNode::set_text(const QString& text)
{
  m_port->set_label(text);
}

}  // namespace omm::nodes
