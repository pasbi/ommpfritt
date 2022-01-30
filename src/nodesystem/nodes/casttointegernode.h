#include "nodesystem/nodes/castnode.h"

namespace omm::nodes
{

class CastToIntegerNode : public CastNode
{
  Q_OBJECT
public:
  explicit CastToIntegerNode(NodeModel& model);
  static const Detail detail;
  static constexpr auto TYPE = "CastToIntegerNode";
};

}  // namespace omm::nodes
