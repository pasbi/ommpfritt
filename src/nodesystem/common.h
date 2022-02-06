#pragma once

#include <QString>
#include <set>

namespace omm::nodes
{
enum class BackendLanguage { Python, GLSL };
}  // namespace omm::nodes

namespace omm::nodes::types
{

constexpr char INVALID_TYPE[] = "Invalid";
constexpr char FLOAT_TYPE[] = "Float";
constexpr char INTEGER_TYPE[] = "Integer";
constexpr char OPTION_TYPE[] = "Option";
constexpr char FLOATVECTOR_TYPE[] = "FloatVector";
constexpr char INTEGERVECTOR_TYPE[] = "IntegerVector";
constexpr char STRING_TYPE[] = "String";
constexpr char COLOR_TYPE[] = "Color";
constexpr char REFERENCE_TYPE[] = "Reference";
constexpr char BOOL_TYPE[] = "Bool";
constexpr char SPLINE_TYPE[] = "Spline";
constexpr auto valid_types = std::array{FLOAT_TYPE, INTEGER_TYPE, OPTION_TYPE, FLOATVECTOR_TYPE,
                                        INTEGERVECTOR_TYPE, STRING_TYPE, COLOR_TYPE, REFERENCE_TYPE,
                                        BOOL_TYPE, SPLINE_TYPE};
bool is_scalar(const QString& type);
bool is_numeric(const QString& type);
bool is_integral(const QString& type);
bool is_vector(const QString& type);
bool is_color(const QString& type);
std::set<QString> supported_types(BackendLanguage language);

}  // namespace omm::nodes::types
