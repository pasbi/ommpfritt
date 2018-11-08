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

std::ostream& AbstractSerializer::serialize(const Scene& scene, std::ostream& ostream)
{
  scene.root().serialize(*this, ROOT_POINTER);
  return ostream;
}

std::istream& AbstractDeserializer::deserialize(Scene& scene, std::istream& istream)
{
  // load data in overridden function
  scene.root().deserialize(*this, AbstractSerializer::ROOT_POINTER);
  return istream;
}

void AbstractSerializer::register_serializers()
{
#define REGISTER_SERIALIZER(TYPE) AbstractSerializer::register_type<TYPE>(#TYPE);
  REGISTER_SERIALIZER(JSONSerializer);
#undef REGISTER_SERIALIZER
}

void AbstractDeserializer::register_deserializers()
{
#define REGISTER_DESERIALIZER(TYPE) AbstractDeserializer::register_type<TYPE>(#TYPE);
  REGISTER_DESERIALIZER(JSONDeserializer);
#undef REGISTER_DESERIALIZER
}

}  // namespace omm
