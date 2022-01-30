#include "nodesystem/nodes/castnode.h"

namespace omm::nodes
{

class CastToFloatNode : public CastNode
{
  Q_OBJECT
public:
  explicit CastToFloatNode(NodeModel& model);
  static const Detail detail;
  static constexpr auto TYPE = "CastToFloatNode";
};

}  // namespace omm::nodes
