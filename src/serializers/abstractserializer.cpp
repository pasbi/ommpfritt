#include "serializers/abstractserializer.h"

#include <numeric>
#include <ostream>
#include <istream>
#include <functional>

#include "scene/scene.h"
#include "tags/tag.h"
#include "objects/object.h"
#include "properties/referenceproperty.h"

namespace
{

auto hash(const omm::AbstractPropertyOwner* ref)
{
  return std::hash<const void*>()(ref);
}

}  // namespace

namespace omm
{


AbstractSerializer::AbstractSerializer(std::ostream& stream)
{
}

AbstractSerializer::~AbstractSerializer()
{
}

void AbstractSerializer::set_value(const AbstractPropertyOwner* ref, const Pointer& pointer)
{
  m_serialized_references.insert(const_cast<AbstractPropertyOwner*>(ref));
  set_value(static_cast<std::size_t>(hash(ref)), pointer);
}

std::set<omm::AbstractPropertyOwner*> AbstractSerializer::serialized_references() const
{
  return m_serialized_references;
}


AbstractDeserializer::AbstractDeserializer(std::istream& stream)
{
}

void AbstractDeserializer::add_references(const std::set<AbstractPropertyOwner*>& references)
{
  for (AbstractPropertyOwner* reference : references) {
    m_id_to_reference.insert(std::make_pair(hash(reference), reference));
  }
}

AbstractDeserializer::~AbstractDeserializer()
{
  // polish reference properties
  for (const auto& property_id : m_reference_property_to_id) {
    auto* property = property_id.first;
    const auto id = property_id.second;
    if (m_id_to_reference.count(id) > 0) {
      auto* reference = id == 0 ? nullptr : m_id_to_reference.at(id);
      property->set(reference);
    }
  }
}

void AbstractDeserializer::register_reference( const std::size_t id,
                                               AbstractPropertyOwner& reference )
{
  m_id_to_reference[id] = &reference;
}

void AbstractDeserializer::register_reference_property(ReferenceProperty& reference_property,
                                                       const std::size_t id )
{
  m_reference_property_to_id[&reference_property] = id;
}

}  // namespace omm
