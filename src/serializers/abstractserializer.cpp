#include "serializers/abstractserializer.h"

#include <numeric>
#include <ostream>
#include <istream>

#include "scene/scene.h"
#include "tags/tag.h"
#include "objects/object.h"
#include "properties/referenceproperty.h"

namespace omm
{


AbstractSerializer::AbstractSerializer(std::ostream& stream)
{
}

AbstractSerializer::~AbstractSerializer()
{
}

AbstractDeserializer::AbstractDeserializer(std::istream& stream)
{
}

AbstractDeserializer::~AbstractDeserializer()
{
  for (const auto& property_id : m_reference_property_to_id) {
    auto* property = property_id.first;
    const auto id = property_id.second;
    auto* reference = id == 0 ? nullptr : m_id_to_reference.at(id);
    property->set_value(reference);
  }
}

void AbstractDeserializer::register_reference( const Serializable::IdType& id,
                                               AbstractPropertyOwner& reference )
{
  m_id_to_reference[id] = &reference;
}

void AbstractDeserializer::register_reference_property(ReferenceProperty& reference_property,
                                                       const Serializable::IdType& id )
{
  m_reference_property_to_id[&reference_property] = id;
}

}  // namespace omm
