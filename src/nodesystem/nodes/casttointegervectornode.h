#include "nodesystem/nodes/castnode.h"

namespace omm::nodes
{

class CastToIntegerVectorNode : public CastNode<types::INTEGERVECTOR_TYPE>
{
  Q_OBJECT
public:
  using CastNode::CastNode;
  static const Detail detail;
  QString type() const override;
  static constexpr auto TYPE = "CastToIntegerVectorNode";
};

}  // namespace omm::nodes
