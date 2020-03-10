#include "nodesystem/ordinaryport.h"
#include "nodesystem/node.h"

namespace omm
{

template<> QString OrdinaryPort<PortType::Input>::data_type() const
{
  return node.input_data_type(*this);
}

template<> QString OrdinaryPort<PortType::Output>::data_type() const
{
  return node.output_data_type(*this);
}

}  // namespace omm
