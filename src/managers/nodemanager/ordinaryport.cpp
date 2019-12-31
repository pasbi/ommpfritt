#include "managers/nodemanager/ordinaryport.h"
#include "managers/nodemanager/node.h"

namespace omm
{

template<> QString OrdinaryPort<PortType::Input>::data_type() const
{
  if (OutputPort* op = connected_output(); op != nullptr) {
    return op->data_type();
  } else {
    return NodeCompilerTypes::INVALID_TYPE;
  }
}

template<> QString OrdinaryPort<PortType::Output>::data_type() const
{
  return node.output_data_type(*this);
}

}  // namespace omm
