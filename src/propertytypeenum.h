#pragma once

#include <array>
#include <string_view>
#include <QtGlobal>
#include "geometry/vec2.h"

namespace omm
{

class AbstractPropertyOwner;
class Color;
class TriggerPropertyDummyValueType;
class SplineType;

enum class Type{
  Invalid, Float, Integer, Option, FloatVector,
  IntegerVector, String, Color, Reference, Bool, Spline, Trigger
};

constexpr bool is_integral(const Type type)
{
  return type == Type::Integer || type == Type::Bool || type == Type::Option || type == Type::Reference;
}

constexpr bool is_scalar(const Type type)
{
  return type == Type::Float || is_integral(type);
}

constexpr bool is_numeric(const Type type)
{
  return is_scalar(type);
}

constexpr bool is_vector(const Type type)
{
  return type == Type::FloatVector || type == Type::IntegerVector;
}

constexpr bool is_color(const Type type)
{
  return type == Type::Color;
}

constexpr auto variant_types = std::array{
      Type::Bool, Type::Float, Type::Color, Type::Integer, Type::IntegerVector,
      Type::FloatVector, Type::Reference, Type::String, Type::Option, Type::Trigger,
      Type::FloatVector, Type::IntegerVector, Type::Spline, Type::Invalid
};

constexpr std::string_view variant_type_name(const Type type) noexcept
{
  switch (type) {
  case Type::Bool:
    return QT_TRANSLATE_NOOP("DataType", "Bool");
  case Type::Float:
    return QT_TRANSLATE_NOOP("DataType", "Float");
  case Type::Color:
    return QT_TRANSLATE_NOOP("DataType", "Color");
  case Type::Integer:
    return QT_TRANSLATE_NOOP("DataType", "Integer");
  case Type::Reference:
    return QT_TRANSLATE_NOOP("DataType", "Reference");
  case Type::String:
    return QT_TRANSLATE_NOOP("DataType", "String");
  case Type::Option:
    return QT_TRANSLATE_NOOP("DataType", "Option");
  case Type::Trigger:
    return QT_TRANSLATE_NOOP("DataType", "Trigger");
  case Type::FloatVector:
    return QT_TRANSLATE_NOOP("DataType", "FloatVector");
  case Type::IntegerVector:
    return QT_TRANSLATE_NOOP("DataType", "IntegerVector");
  case Type::Spline:
    return QT_TRANSLATE_NOOP("DataType", "Spline");
  case Type:: Invalid:
    return QT_TRANSLATE_NOOP("DataType", "Invalid");
  }
  Q_UNREACHABLE();
  return {};
}

Type get_variant_type(const std::string_view& v);

template<typename T> constexpr Type get_variant_type() noexcept
{
  if constexpr (std::is_same_v<T, bool>) {
    return Type::Bool;
  } else if constexpr (std::is_same_v<T, double>) {
    return Type::Float;
  } else if constexpr (std::is_same_v<T, Color>) {
    return Type::Color;
  } else if constexpr (std::is_same_v<T, int>) {
    return Type::Integer;
  } else if constexpr (std::is_same_v<T, AbstractPropertyOwner*>) {
    return Type::Reference;
  } else if constexpr (std::is_same_v<T, QString>) {
    return Type::String;
  } else if constexpr (std::is_same_v<T, std::size_t>) {
    return Type::Option;
  } else if constexpr (std::is_same_v<T, TriggerPropertyDummyValueType>) {
    return Type::Trigger;
  } else if constexpr (std::is_same_v<T, Vec2f>) {
    return Type::FloatVector;
  } else if constexpr (std::is_same_v<T, Vec2i>) {
    return Type::IntegerVector;
  } else if constexpr (std::is_same_v<T, SplineType>) {
    return Type::Spline;
  } else {
    return Type::Invalid;
  }
}

}  // namespace
