//#include "animation/track.h"
//#include "animation/track.h"
//#include "properties/property.h"

//namespace omm
//{

//std::string map_property_to_fcurve_type(const std::string &property_type)
//{
//  // <TYPE>Property -> <TYPE>FCurve
//  static const std::string property_suffix = "property";
//  static const std::string fcurve_suffix = "fcurve";

//  assert(property_type.size() > property_suffix.size());
//  const std::string type = property_type.substr(0, property_type.size() - property_suffix.size());
//  return type + fcurve_suffix;
//}

//}  // namespace

//namespace omm
//{

//Track::Track(Property* property)
//{
//  if (property != nullptr) {
//    m_fcurve = AbstractFCurve::make(map_property_to_fcurve_type(property->type()));
//  }
//}

//Track::~Track()
//{

//}

//void Track::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& pointer)
//{
//  const auto fcurve_pointer = make_pointer(pointer, FCURVE_POINTER);
//  const std::string type = deserializer.get_string(make_pointer(fcurve_pointer,
//                                                                AbstractFCurve::TYPE_KEY));
//  m_fcurve = AbstractFCurve::make(type);
//  m_fcurve->deserialize(deserializer, fcurve_pointer);
//}

//void Track::serialize(AbstractSerializer& serializer, const Serializable::Pointer& pointer) const
//{
//  const auto fcurve_pointer = make_pointer(pointer, FCURVE_POINTER);
//  serializer.set_value(m_fcurve->type(), make_pointer(fcurve_pointer, AbstractFCurve::TYPE_KEY));
//  m_fcurve->serialize(serializer, fcurve_pointer);
//}

//AbstractFCurve &Track::fcurve() const
//{
//  assert(m_fcurve != nullptr);
//  return *m_fcurve;
//}

//}  // namespace omm
