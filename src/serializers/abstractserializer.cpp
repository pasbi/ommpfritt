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


AbstractSerializer::AbstractSerializer(std::ostream&)
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


AbstractDeserializer::AbstractDeserializer(std::istream&)
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

template<> bool AbstractDeserializer::get<bool>(const Pointer& pointer)
{
  return get_bool(pointer);
}

template<> int AbstractDeserializer::get<int>(const Pointer& pointer)
{
  return get_int(pointer);
}

template<> double AbstractDeserializer::get<double>(const Pointer& pointer)
{
  return get_double(pointer);
}

template<> std::string AbstractDeserializer::get<std::string>(const Pointer& pointer)
{
  return get_string(pointer);
}

template<> std::size_t AbstractDeserializer::get<std::size_t>(const Pointer& pointer)
{
  return get_size_t(pointer);
}

template<> Color AbstractDeserializer::get<Color>(const Pointer& pointer)
{
  return get_color(pointer);
}

template<> Vec2f AbstractDeserializer::get<Vec2f>(const Pointer& pointer)
{
  return get_vec2f(pointer);
}

template<> Vec2i AbstractDeserializer::get<Vec2i>(const Pointer& pointer)
{
  return get_vec2i(pointer);
}

template<> PolarCoordinates AbstractDeserializer::get<PolarCoordinates>(const Pointer& pointer)
{
  return get_polarcoordinates(pointer);
}

}  // namespace omm
