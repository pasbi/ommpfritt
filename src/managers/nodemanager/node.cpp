#include "managers/nodemanager/node.h"
#include "common.h"

namespace omm
{

Node::Node(Scene* scene) : PropertyOwner(scene)
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

void Node::serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const
{
  AbstractPropertyOwner::serialize(serializer, root);
  serializer.set_value(Vec2f(pos()), make_pointer(root, POS_KEY));
}

void Node::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root)
{
  AbstractPropertyOwner::deserialize(deserializer, root);
  m_pos = deserializer.get_vec2f(make_pointer(root, POS_KEY)).to_pointf();
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

}  // namespace omm
