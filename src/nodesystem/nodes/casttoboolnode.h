#include "nodesystem/nodes/castnode.h"

namespace omm::nodes
{

class CastToBoolNode : public CastNode<types::BOOL_TYPE>
{
  Q_OBJECT
public:
  using CastNode::CastNode;
  static const Detail detail;
  QString type() const override;
  static constexpr auto TYPE = "CastToBoolNode";
};

}  // namespace omm::nodes
