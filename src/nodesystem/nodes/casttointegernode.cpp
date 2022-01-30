#include "nodesystem/nodes/casttointegernode.h"

namespace omm::nodes
{

const Node::Detail CastToIntegerNode::detail {
  .definitions = {{BackendLanguage::GLSL, ""}},
  .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Conversion")},
};

QString CastToIntegerNode::type() const
{
  return TYPE;
}

}  // namespace omm::nodes
