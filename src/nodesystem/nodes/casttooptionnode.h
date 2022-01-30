#include "nodesystem/nodes/castnode.h"

namespace omm::nodes
{

class CastToOptionNode : public CastNode
{
  Q_OBJECT
public:
  explicit CastToOptionNode(NodeModel& model);
  static const Detail detail;
  static constexpr auto TYPE = "CastToOptionNode";
};

}  // namespace omm::nodes
