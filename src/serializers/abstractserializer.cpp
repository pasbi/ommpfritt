#include "serializers/abstractserializer.h"

#include <numeric>
#include <ostream>
#include <istream>

#include "scene/scene.h"
#include "tags/tag.h"
#include "objects/object.h"

#include "serializers/jsonserializer.h"

namespace omm
{


AbstractSerializer::AbstractSerializer(std::ostream& stream)
{
}

AbstractSerializer::~AbstractSerializer()
{
}

void AbstractSerializer::register_serializers()
{
#define REGISTER_SERIALIZER(TYPE) AbstractSerializer::register_type<TYPE>(#TYPE);
  REGISTER_SERIALIZER(JSONSerializer);
#undef REGISTER_SERIALIZER
}


AbstractDeserializer::AbstractDeserializer(std::istream& stream)
{
}

AbstractDeserializer::~AbstractDeserializer()
{
}

void AbstractDeserializer::register_deserializers()
{
#define REGISTER_DESERIALIZER(TYPE) AbstractDeserializer::register_type<TYPE>(#TYPE);
  REGISTER_DESERIALIZER(JSONDeserializer);
#undef REGISTER_DESERIALIZER
}

}  // namespace omm
