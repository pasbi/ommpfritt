#include "nodesystem/nodes/castnode.h"

namespace omm::nodes
{

class CastToFloatVectorNode : public CastNode<types::FLOATVECTOR_TYPE>
{
  Q_OBJECT
public:
  using CastNode::CastNode;
  static const Detail detail;
  QString type() const override;
  static constexpr auto TYPE = "CastToFloatVectorNode";
};

}  // namespace omm::nodes
