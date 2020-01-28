#include "managers/nodemanager/nodes/mathnode.h"
#include "managers/nodemanager/nodes/spynode.h"
#include "managers/nodemanager/nodes/constantnode.h"
#include "managers/nodemanager/nodes/referencenode.h"
#include "managers/nodemanager/nodes/composenode.h"
#include "managers/nodemanager/nodes/decomposenode.h"
#include "managers/nodemanager/nodes/decomposecolornode.h"
#include "managers/nodemanager/nodes/composecolornode.h"
#include "managers/nodemanager/nodes/fragmentnode.h"
#include "managers/nodemanager/nodes/vertexnode.h"
#include "managers/nodemanager/nodes/colorconvertnode.h"
#include "managers/nodemanager/nodes/functionnode.h"
#include "managers/nodemanager/nodes/function2node.h"

namespace omm
{

void register_nodes()
{
#define REGISTER_NODE(TYPE) Node::register_type<TYPE>(#TYPE); \
  Node::m_details[#TYPE] = &TYPE::detail

  REGISTER_NODE(ReferenceNode);
  REGISTER_NODE(MathNode);
  REGISTER_NODE(SpyNode);
  REGISTER_NODE(ConstantNode);
  REGISTER_NODE(DecomposeNode);
  REGISTER_NODE(ComposeNode);
  REGISTER_NODE(FragmentNode);
  REGISTER_NODE(DecomposeColorNode);
  REGISTER_NODE(ComposeColorNode);
  REGISTER_NODE(VertexNode);
  REGISTER_NODE(ColorConvertNode);
  REGISTER_NODE(FunctionNode);
  REGISTER_NODE(Function2Node);
#undef REGISTER_NODE
}

}  // namespace omm
