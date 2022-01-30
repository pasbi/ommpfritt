#include "nodesystem/nodes/casttofloatvectornode.h"

namespace omm::nodes
{

const Node::Detail CastToFloatVectorNode::detail {
  .definitions = {{BackendLanguage::GLSL, ""}},
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Conversion")},
    };

CastToFloatVectorNode::CastToFloatVectorNode(NodeModel& model)
    : CastNode(model, TYPE, types::FLOATVECTOR_TYPE)
{
}

}  // namespace omm::nodes
