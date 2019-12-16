#include "managers/nodemanager/nodes/constantnode.h"
#include "variant.h"
#include "scene/scene.h"
#include "properties/optionsproperty.h"

namespace omm
{

ConstantNode::ConstantNode(Scene* scene)
  : Node(scene)
{
}

void ConstantNode::on_property_value_changed(Property* property)
{
}

}  // namespace
