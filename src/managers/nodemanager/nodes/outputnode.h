#include "managers/nodemanager/node.h"

namespace omm
{

class OutputNode : public Node
{
public:
  explicit OutputNode(Scene* scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "OutputNode");
  QString type() const override { return TYPE; }
};

}  // namespace omm
