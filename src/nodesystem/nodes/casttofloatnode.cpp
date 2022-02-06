#include "nodesystem/nodes/casttofloatnode.h"

namespace omm::nodes
{

const Node::Detail CastToFloatNode::detail {
  .definitions = {{BackendLanguage::GLSL, ""}},
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Conversion")},
    };

CastToFloatNode::CastToFloatNode(NodeModel& model)
    : CastNode(model, TYPE, Type::Float)
{
}

}  // namespace omm::nodes
