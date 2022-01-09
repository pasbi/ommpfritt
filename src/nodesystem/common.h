#pragma once

#include <QString>
#include <set>

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
