#include "nodesystem/nodes/castnode.h"

namespace omm::nodes
{

class CastToIntegerVectorNode : public CastNode
{
  Q_OBJECT
public:
  explicit CastToIntegerVectorNode(NodeModel& model);
  static const Detail detail;
  static constexpr auto TYPE = "CastToIntegerVectorNode";
};

}  // namespace omm::nodes
