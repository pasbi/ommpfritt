#include "nodesystem/nodes/casttooptionnode.h"

namespace omm::nodes
{

const Node::Detail CastToOptionNode::detail {
  .definitions = {{BackendLanguage::GLSL, ""}},
  .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Conversion")},
};

QString CastToOptionNode::type() const
{
  return TYPE;
}

}  // namespace omm::nodes
