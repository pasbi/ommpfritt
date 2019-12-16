#include "managers/nodemanager/node.h"

namespace omm
{

class InputNode : public Node
{
public:
  explicit InputNode(Scene* scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "InputNode");
  QString type() const override { return TYPE; }

};

}  // namespace omm
