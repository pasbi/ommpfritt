#include "serializers/abstractserializer.h"
#include "serializers/jsonserializer.h"

namespace omm
{

void register_serializers()
{
#define REGISTER_SERIALIZER(TYPE) AbstractSerializer::register_type<TYPE>(#TYPE);
  REGISTER_SERIALIZER(JSONSerializer);
#undef REGISTER_SERIALIZER
}

void register_deserializers()
{
#define REGISTER_DESERIALIZER(TYPE) AbstractDeserializer::register_type<TYPE>(#TYPE);
  REGISTER_DESERIALIZER(JSONDeserializer);
#undef REGISTER_DESERIALIZER
}

}  // namespace omm
