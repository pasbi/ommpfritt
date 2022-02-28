#include "serializers/serializerworker.h"
#include "aspects/propertyowner.h"
#include "aspects/serializable.h"


namespace omm::serialization
{

void SerializerWorker::set_value(const AbstractPropertyOwner* ref)
{
  set_value(ref == nullptr ? 0 : ref->id());
}

void SerializerWorker::set_value(const variant_type& variant)
{
  std::visit([this](auto&& arg) { set_value(arg); }, variant);
}

}  // namespace omm::serialization
