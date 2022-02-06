#include "nodesystem/common.h"
#include "properties/property.h"
#include "properties/stringproperty.h"
#include "properties/triggerproperty.h"

namespace omm::nodes::types
{

bool is_integral(const QString& type)
{
  return type == BOOL_TYPE || type == INTEGER_TYPE || type == OPTION_TYPE;
}

bool is_scalar(const QString& type)
{
  return is_integral(type) || type == FLOAT_TYPE;
}

bool is_vector(const QString& type)
{
  return type == INTEGERVECTOR_TYPE || type == FLOATVECTOR_TYPE;
}

bool is_color(const QString& type)
{
  return type == COLOR_TYPE;
}

std::set<QString> supported_types(BackendLanguage language)
{
  switch (language) {
  case BackendLanguage::Python:
    return Property::keys();
  case BackendLanguage::GLSL:
    return ::filter_if(Property::keys(), [](const auto& c) {
      return c != StringProperty::TYPE() && c != TriggerProperty::TYPE();
    });
  default:
    Q_UNREACHABLE();
    return std::set<QString>();
  }
}

bool is_numeric(const QString& type)
{
  return is_scalar(type);
}


}  // namespace omm::nodes::types
