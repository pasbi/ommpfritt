#include "nodesystem/nodes/castnode.h"

namespace omm::nodes
{

class CastToFloatNode : public CastNode<types::FLOAT_TYPE>
{
  Q_OBJECT
public:
  using CastNode::CastNode;
  static const Detail detail;
  QString type() const override;
  static constexpr auto TYPE = "CastToFloatNode";
};

}  // namespace omm::nodes
