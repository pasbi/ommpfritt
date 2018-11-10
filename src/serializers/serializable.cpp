#include "serializers/jsonserializer.h"

#include <functional>

#include "serializers/serializable.h"

namespace
{
constexpr auto ID_POINTER = "id";
}  // namespace

namespace omm
{

void Serializable::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  const auto id_pointer = make_pointer(root, ID_POINTER);
  serializer.set_value(identify(), id_pointer);
}

void Serializable::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  const auto id_pointer = make_pointer(root, ID_POINTER);
  const auto id = deserializer.get_id(id_pointer);
  deserializer.register_reference(id, *this);
}

Serializable::IdType Serializable::identify() const
{
  return std::hash<const void*>()(this);
}

}  // namespace omm
