#include "managers/nodemanager/ordinaryport.h"
#include "managers/nodemanager/node.h"

namespace omm
{

template<> QString OrdinaryPort<PortType::Output>::data_type() const
{
  return node.output_data_type(*this);
}

}  // namespace omm
