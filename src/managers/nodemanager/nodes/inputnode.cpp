#include "managers/nodemanager/nodes/inputnode.h"

namespace omm
{

InputNode::InputNode(Scene* scene) : Node(scene)
{
  add_port<OutputPort>(QObject::tr("pos"));
}

}  // namespace omm
