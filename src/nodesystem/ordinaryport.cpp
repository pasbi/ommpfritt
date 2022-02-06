#include "nodesystem/ordinaryport.h"
#include "nodesystem/node.h"

namespace omm::nodes
{

template<> Type OrdinaryPort<PortType::Input>::data_type() const
{
  return node.input_data_type(*this);
}

template<> Type OrdinaryPort<PortType::Output>::data_type() const
{
  return node.output_data_type(*this);
}

}  // namespace omm::nodes
