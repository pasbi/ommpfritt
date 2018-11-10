#include "serializers/abstractserializer.h"

#include <numeric>
#include <ostream>
#include <istream>

#include "scene/scene.h"
#include "tags/tag.h"
#include "objects/object.h"
#include "properties/referenceproperty.h"

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
  LOG(INFO) << m_reference_property_to_id.size();
  LOG(INFO) << m_id_to_reference.size();
  for (const auto& property_id : m_reference_property_to_id) {
    auto* property = property_id.first;
    const auto id = property_id.second;
    auto* reference = id == 0 ? nullptr : m_id_to_reference.at(id);
    property->set_value(reference);
  }
}

void AbstractDeserializer::register_deserializers()
{
#define REGISTER_DESERIALIZER(TYPE) AbstractDeserializer::register_type<TYPE>(#TYPE);
  REGISTER_DESERIALIZER(JSONDeserializer);
#undef REGISTER_DESERIALIZER
}


void AbstractDeserializer::register_reference( const Serializable::IdType& id,
                                               Serializable& reference )
{
  m_id_to_reference[id] = &reference;
}

void AbstractDeserializer::register_reference_property(ReferenceProperty& reference_property,
                                                       const Serializable::IdType& id )
{
  m_reference_property_to_id[&reference_property] = id;
}

}  // namespace omm
