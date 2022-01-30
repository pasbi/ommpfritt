#include "nodesystem/nodes/castnode.h"

namespace omm::nodes
{

class CastToFloatVectorNode : public CastNode
{
  Q_OBJECT
public:
  explicit CastToFloatVectorNode(NodeModel& model);
  static const Detail detail;
  static constexpr auto TYPE = "CastToFloatVectorNode";
};

}  // namespace omm::nodes
