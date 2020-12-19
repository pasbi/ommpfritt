#include "serializers/abstractserializer.h"

#include <functional>
#include <istream>
#include <numeric>
#include <ostream>

#include "objects/object.h"
#include "properties/referenceproperty.h"
#include "scene/scene.h"
#include "tags/tag.h"

namespace omm
{
void AbstractSerializer::set_value(const AbstractPropertyOwner* ref, const Pointer& pointer)
{
  set_value(ref == nullptr ? 0 : ref->id(), pointer);
}

void AbstractSerializer::set_value(const variant_type& variant, const Pointer& pointer)
{
  std::visit([this, pointer](auto&& arg) { set_value(arg, pointer); }, variant);
}

void AbstractSerializer::set_value(const Serializable& serializable,
                                   const AbstractSerializer::Pointer& pointer)
{
  serializable.serialize(*this, pointer);
}

void AbstractDeserializer::add_references(const std::set<AbstractPropertyOwner*>& references)
{
  for (AbstractPropertyOwner* reference : references) {
    register_reference(reference->id(), *reference);
  }
}

void AbstractDeserializer::polish()
{
  // polish reference properties
  for (ReferencePolisher* polisher : m_reference_polishers) {
    polisher->update_references(m_id_to_reference);
  }
}

void AbstractDeserializer::register_reference(const std::size_t id,
                                              AbstractPropertyOwner& reference)
{
  const auto it = m_id_to_reference.find(id);
  if (it == m_id_to_reference.end()) {
    m_id_to_reference.insert({id, &reference});
  } else {
    if (it->second != &reference) {
      const QString msg = "Ambiguous id: %1.";
      // Unfortunately, it's not clear if the objects have already been deserialized.
      // Hence it does not make much sense to print their names.
      // User must search for the id in the json file to resolve the issue.
      throw DeserializeError(msg.arg(id).toStdString().c_str());
    }
  }
}

void AbstractDeserializer::register_reference_polisher(ReferencePolisher& polisher)
{
  m_reference_polishers.insert(&polisher);
}

void AbstractDeserializer::get(Serializable& serializable,
                               const AbstractDeserializer::Pointer& pointer)
{
  serializable.deserialize(*this, pointer);
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

template<> QString AbstractDeserializer::get<QString>(const Pointer& pointer)
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

template<> SplineType AbstractDeserializer::get<SplineType>(const Pointer& pointer)
{
  return get_spline(pointer);
}

template<>
TriggerPropertyDummyValueType
AbstractDeserializer ::get<TriggerPropertyDummyValueType>(const Pointer& pointer)
{
  return get_trigger_dummy_value(pointer);
}

variant_type AbstractDeserializer::get(const AbstractDeserializer::Pointer& pointer,
                                       const QString& type)
{
  if (type == "Bool") {
    return get<bool>(pointer);
  } else if (type == "Integer") {
    return get<int>(pointer);
  } else if (type == "Float") {
    return get<double>(pointer);
  } else if (type == "String") {
    return get<QString>(pointer);
  } else if (type == "Options") {
    return get<std::size_t>(pointer);
  } else if (type == "Color") {
    return get<Color>(pointer);
  } else if (type == "FloatVector") {
    return get<Vec2f>(pointer);
  } else if (type == "IntegerVector") {
    return get<Vec2i>(pointer);
  } else if (type == "SplineType") {
    return get<SplineType>(pointer);
  } else if (type == "Reference") {
    Q_UNREACHABLE();
    // Reference values must be treated specially.
    // See ReferenceProperty for details.
    return nullptr;
  } else if (type == "Trigger") {
    return get<TriggerPropertyDummyValueType>(pointer);
  } else {
    LERROR << "Unknown variant type: '" << type << "'.";
    Q_UNREACHABLE();
  }
}

}  // namespace omm
