#include "nodesystem/nodes/casttointegernode.h"

namespace omm::nodes
{

const Node::Detail CastToIntegerNode::detail {
  .definitions = {{BackendLanguage::GLSL, ""}},
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Conversion")},
    };

CastToIntegerNode::CastToIntegerNode(NodeModel& model)
    : CastNode(model, TYPE, types::INTEGER_TYPE)
{
}


}  // namespace omm::nodes
