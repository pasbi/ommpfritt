#include "managers/nodemanager/nodes/mathnode.h"
#include "managers/nodemanager/nodes/spynode.h"
#include "managers/nodemanager/nodes/constantnode.h"
#include "managers/nodemanager/nodes/referencenode.h"
#include "managers/nodemanager/nodes/composenode.h"
#include "managers/nodemanager/nodes/decomposenode.h"

namespace omm
{

void register_nodes()
{
#define REGISTER_NODE(TYPE) Node::register_type<TYPE>(#TYPE)
  REGISTER_NODE(ReferenceNode);
  REGISTER_NODE(MathNode);
  REGISTER_NODE(SpyNode);
  REGISTER_NODE(ConstantNode);
  REGISTER_NODE(DecomposeNode);
  REGISTER_NODE(ComposeNode);
#undef REGISTER_NODE
}

}  // namespace omm
