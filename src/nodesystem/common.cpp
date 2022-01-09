#include "nodesystem/common.h"
#include "properties/property.h"
#include "properties/stringproperty.h"
#include "properties/triggerproperty.h"
#include "nodesystem/nodecompilerglsl.h"

namespace omm::nodes::types
{

bool is_integral(const QString& type)
{
  return type == BOOL_TYPE || type == INTEGER_TYPE || type == OPTION_TYPE;
}

bool is_numeric(const QString& type)
{
  return is_integral(type) || type == FLOAT_TYPE;
}

bool is_vector(const QString& type)
{
  return type == INTEGERVECTOR_TYPE || type == FLOATVECTOR_TYPE;
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

}  // namespace omm::nodes::types


namespace omm::nodes
{

QString codegeneration::indent(QString code, int level)
{
  const QString base_indentation = "    ";
  const auto indentation = base_indentation.repeated(level);
  return indentation + code.replace('\n', "\n" + indentation);
}

QString codegeneration::translate_type_glsl(std::string_view type)
{
  return NodeCompilerGLSL::translate_type(QString::fromStdString(std::string{type}));
}

}  // namespace omm::nodes
