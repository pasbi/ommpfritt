#include "nodesystem/nodes/mathnode.h"
#include "nodesystem/nodes/interpolatenode.h"
#include "nodesystem/nodes/spynode.h"
#include "nodesystem/nodes/constantnode.h"
#include "nodesystem/nodes/referencenode.h"
#include "nodesystem/nodes/composenode.h"
#include "nodesystem/nodes/decomposenode.h"
#include "nodesystem/nodes/decomposecolornode.h"
#include "nodesystem/nodes/composecolornode.h"
#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/nodes/vertexnode.h"
#include "nodesystem/nodes/colorconvertnode.h"
#include "nodesystem/nodes/functionnode.h"
#include "nodesystem/nodes/function2node.h"
#include "nodesystem/nodes/linepatternnode.h"

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
  REGISTER_NODE(LinePatternNode);
  REGISTER_NODE(InterpolateNode);
#undef REGISTER_NODE
}

}  // namespace omm
