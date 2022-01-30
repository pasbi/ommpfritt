#include "nodesystem/nodes/castnode.h"

namespace omm::nodes
{

class CastToOptionNode : public CastNode<types::OPTION_TYPE>
{
  Q_OBJECT
public:
  using CastNode::CastNode;
  static const Detail detail;
  QString type() const override;
  static constexpr auto TYPE = "CastToOptionNode";
};

}  // namespace omm::nodes
