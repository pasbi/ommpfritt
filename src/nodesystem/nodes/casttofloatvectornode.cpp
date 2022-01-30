#include "nodesystem/nodes/casttofloatvectornode.h"

namespace omm::nodes
{

const Node::Detail CastToFloatVectorNode::detail {
  .definitions = {{BackendLanguage::GLSL, ""}},
  .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Conversion")},
};

QString CastToFloatVectorNode::type() const
{
  return TYPE;
}

}  // namespace omm::nodes
