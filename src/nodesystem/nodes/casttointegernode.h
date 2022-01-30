#include "nodesystem/nodes/castnode.h"

namespace omm::nodes
{

class CastToIntegerNode : public CastNode<types::INTEGER_TYPE>
{
  Q_OBJECT
public:
  using CastNode::CastNode;
  static const Detail detail;
  QString type() const override;
  static constexpr auto TYPE = "CastToIntegerNode";
};

}  // namespace omm::nodes
