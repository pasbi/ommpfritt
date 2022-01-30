#include "nodesystem/nodes/casttofloatnode.h"

namespace omm::nodes
{

const Node::Detail CastToFloatNode::detail {
  .definitions = {{BackendLanguage::GLSL, ""}},
  .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Conversion")},
};

QString CastToFloatNode::type() const
{
  return TYPE;
}

}  // namespace omm::nodes
