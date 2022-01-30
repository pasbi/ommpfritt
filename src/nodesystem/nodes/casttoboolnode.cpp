#include "nodesystem/nodes/casttoboolnode.h"

namespace omm::nodes
{

const Node::Detail CastToBoolNode::detail {
  .definitions = {{BackendLanguage::GLSL, ""}},
  .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Conversion")},
};

QString CastToBoolNode::type() const
{
  return TYPE;
}

}  // namespace omm::nodes
