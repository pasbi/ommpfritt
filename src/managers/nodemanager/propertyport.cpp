#include "managers/nodemanager/propertyport.h"

namespace
{

QString data_type(const omm::Property* property)
{
  if (property == nullptr) {
    return "None";
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
    return op->data_type();
  } else {
    return ::data_type(property());
  }
}

template<> QString PropertyPort<PortType::Output>::data_type() const
{
  return ::data_type(property());
}

}  // namespace omm
