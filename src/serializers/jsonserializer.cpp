#include "serializers/jsonserializer.h"

namespace omm
{

// Property::Property(const nlohmann::json& json)
//   : m_label(json["label"].get<std::string>())
//   , m_category(json["category"].get<std::string>())
// {\




// template<typename ValueT> nlohmann::json TypedProperty<ValueT>::to_json() const
// {
//   return TypedProperty<ValueT>::value();
// }
// template<> nlohmann::json TypedProperty<ObjectTransformation>::to_json() const
// {
//   return TypedProperty<ObjectTransformation>::value().to_json();
// }
// 
// template<> nlohmann::json TypedProperty<Object*>::to_json() const
// {
//   return value() ? nlohmann::json(value()->id()) : nlohmann::json();
// }

}  // namespace omm
