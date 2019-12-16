#include "managers/nodemanager/nodes/spynode.h"

namespace omm
{

SpyNode::SpyNode(Scene* scene) : Node(scene)
{
  add_port<InputPort>("");
}

}  // namespace
