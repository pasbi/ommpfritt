#include "nodesystem/nodes/casttointegervectornode.h"

namespace omm::nodes
{

const Node::Detail CastToIntegerVectorNode::detail {
  .definitions = {{BackendLanguage::GLSL, ""}},
  .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Conversion")},
};

QString CastToIntegerVectorNode::type() const
{
  return TYPE;
}

}  // namespace omm::nodes
