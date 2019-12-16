#include "managers/nodemanager/nodes/gradientnode.h"
#include "managers/nodemanager/nodes/constantnode.h"
#include "managers/nodemanager/nodes/spynode.h"
#include "managers/nodemanager/nodes/outputnode.h"
#include "managers/nodemanager/nodes/inputnode.h"

namespace omm
{

void register_nodes()
{
#define REGISTER_NODE(TYPE) Node::register_type<TYPE>(#TYPE)
  REGISTER_NODE(GradientNode);
  REGISTER_NODE(OutputNode);
  REGISTER_NODE(InputNode);
  REGISTER_NODE(SpyNode);
  REGISTER_NODE(ConstantNode);
#undef REGISTER_NODE
}

}  // namespace omm
