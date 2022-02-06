#include "nodesystem/propertyport.h"
#include "nodesystem/node.h"

namespace
{

omm::Type data_type(const omm::Property* property)
{
  if (property == nullptr) {
    return omm::Type::Invalid;
  } else {
    return property->data_type();
  }
}

}  // namespace

namespace omm::nodes
{
template<> Type PropertyPort<PortType::Input>::data_type() const
{
  return ::data_type(property());
}

template<> Type PropertyPort<PortType::Output>::data_type() const
{
  return ::data_type(property());
}

void PropertyInputPort::connect(OutputPort* port)
{
  if (Property* property = this->property(); property != nullptr) {
    property->set_enabledness(port == nullptr);
  }

  PropertyPort<PortType::Input>::connect(port);
}

}  // namespace omm::nodes
