#include "managers/nodemanager/propertyport.h"
#include "managers/nodemanager/node.h"

namespace
{

QString data_type(const omm::Property* property)
{
  if (property == nullptr) {
    return omm::NodeCompilerTypes::INVALID_TYPE;
  } else {
    return property->data_type();
  }
}

}  // namespace

namespace omm
{

template<> QString PropertyPort<PortType::Input>::data_type() const
{
  if (OutputPort* op = connected_output(); op != nullptr) {
    const QString type_candidate = op->data_type();
    return op->data_type();
  } else {
    return ::data_type(property());
  }
}

template<> QString PropertyPort<PortType::Output>::data_type() const
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

}  // namespace omm
