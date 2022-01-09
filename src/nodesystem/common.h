#pragma once

#include <set>
#include <QString>
#include <QStringList>

namespace omm::nodes
{
enum class BackendLanguage { Python, GLSL };
}  // namespace omm::nodes

namespace omm::nodes::types
{

constexpr auto INVALID_TYPE = "Invalid";
constexpr auto FLOAT_TYPE = "Float";
constexpr auto INTEGER_TYPE = "Integer";
constexpr auto OPTION_TYPE = "Option";
constexpr auto FLOATVECTOR_TYPE = "FloatVector";
constexpr auto INTEGERVECTOR_TYPE = "IntegerVector";
constexpr auto STRING_TYPE = "String";
constexpr auto COLOR_TYPE = "Color";
constexpr auto REFERENCE_TYPE = "Reference";
constexpr auto BOOL_TYPE = "Bool";
constexpr auto SPLINE_TYPE = "Spline";
constexpr auto valid_types = std::array{FLOAT_TYPE, INTEGER_TYPE, OPTION_TYPE, FLOATVECTOR_TYPE,
                                        INTEGERVECTOR_TYPE, STRING_TYPE, COLOR_TYPE, REFERENCE_TYPE,
                                        BOOL_TYPE, SPLINE_TYPE};
bool is_numeric(const QString& type);
bool is_integral(const QString& type);
bool is_vector(const QString& type);
std::set<QString> supported_types(BackendLanguage language);

}  // namespace omm::nodes::types

namespace omm::nodes::codegeneration
{

template<auto language, typename Ts, typename F, typename G>
QString if_else_chain(const Ts& values, const F& generate_condition, const G& generate_return_value)
{
  QStringList clauses;
  clauses.reserve(values.size());
  for (std::size_t i = 0; i < values.size(); ++i) {
    const auto& val = values.at(i);
    const auto condition = [cond=generate_condition(i, val)]() {
      if constexpr (language == BackendLanguage::GLSL) {
        return "if (" + cond + ")";
      } else {
        return "if " + cond + ":";
      }
    }();
    const auto return_value = [ret=generate_return_value(val)]() {
      if constexpr (language == BackendLanguage::GLSL) {
        return QString{" {\n  return %1;\n}"}.arg(ret);
      } else {
        return QString{"\n  return %1\n"}.arg(ret);
      }
    }();
    clauses.push_back(condition + return_value);
  }

  const auto else_separator = language == BackendLanguage::GLSL ? " else " : " el";
  return clauses.join(else_separator);
}

QString indent(QString code, int level);
QString translate_type_glsl(std::string_view type);

template<typename Overload>
QString generate_overload(QString template_definition, const Overload& overload)
{
  for (const auto& type : overload.types()) {
    template_definition = template_definition.arg(translate_type_glsl(type));
  }
  return template_definition;
}

template<typename Overloads>
QString overload(const QString& definition_template, const Overloads& overloads)
{
  QStringList definitions;
  definitions.reserve(overloads.size());
  for (const auto& overload : overloads) {
    definitions.push_back(generate_overload(definition_template, overload));
  }
  return definitions.join("\n");
}

}  // namespace omm::nodes::codegeneration
