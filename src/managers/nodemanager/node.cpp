#include "managers/nodemanager/node.h"
#include "common.h"

namespace omm
{

Node::Node()
{
}

Node::~Node()
{
}

std::set<Port*> Node::ports() const
{
  return ::transform<Port*>(m_ports, [](const std::unique_ptr<Port>& p) {
    return p.get();
  });
}

void Node::set_model(NodeModel* model)
{
  m_model = model;
}

void Node::set_pos(const QPointF& pos)
{
  m_pos = pos;
  Q_EMIT pos_changed();
}

QPointF Node::pos() const
{
  return m_pos;
}

//std::set<Port*> Node::ports() const
//{
//  std::set<Port*> ports;
//  for (auto&& op : m_outputs) {
//    ports.insert(op.get());
//  }
//  for (auto&& ip : m_inputs) {
//    ports.insert(ip.get());
//  }
//  return ports;
//}



}  // namespace omm
