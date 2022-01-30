#include "nodesystem/nodes/casttointegervectornode.h"

namespace omm::nodes
{

const Node::Detail CastToIntegerVectorNode::detail {
  .definitions = {{BackendLanguage::GLSL, ""}},
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Conversion")},
    };

CastToIntegerVectorNode::CastToIntegerVectorNode(NodeModel& model)
    : CastNode(model, TYPE, types::INTEGERVECTOR_TYPE)
{

}

}  // namespace omm::nodes
