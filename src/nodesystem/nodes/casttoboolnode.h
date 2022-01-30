#include "nodesystem/nodes/castnode.h"

namespace omm::nodes
{

class CastToBoolNode : public CastNode
{
  Q_OBJECT
public:
  CastToBoolNode(NodeModel& model);
  static const Detail detail;
  static constexpr auto TYPE = "CastToBoolNode";
};

}  // namespace omm::nodes
