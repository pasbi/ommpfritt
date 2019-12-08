#include "managers/nodemanager/node.h"

namespace omm
{

class GradientNode : public Node
{
public:
  explicit GradientNode(Scene* scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "GradientNode");
  QString type() const override { return TYPE; }
};

}  // namespace omm
