#include "managers/nodemanager/node.h"

namespace omm
{

class SpyNode : public Node
{
public:
  explicit SpyNode(Scene* scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SpyNode");
  QString type() const override { return TYPE; }
};

}  // namespace omm
