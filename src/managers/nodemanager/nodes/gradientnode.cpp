#include "managers/nodemanager/nodes/gradientnode.h"

namespace omm
{

GradientNode::GradientNode(Scene* scene) : Node(scene)
{
  add_port<InputPort>("pos");
  add_port<OutputPort>("color");
}

}  // namespace
