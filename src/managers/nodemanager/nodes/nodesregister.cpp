#include "managers/nodemanager/nodes/gradientnode.h"

namespace omm
{

void register_nodes()
{
#define REGISTER_NODE(TYPE) Node::register_type<TYPE>(#TYPE)
  REGISTER_NODE(GradientNode);
#undef REGISTER_NODE
}

}  // namespace omm
