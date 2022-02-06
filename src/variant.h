#pragma once

#include "color/color.h"
#include "geometry/vec2.h"
#include "logging.h"
#include "splinetype.h"
#include <QString>
#include <variant>

namespace omm
{
class AbstractPropertyOwner;

class TriggerPropertyDummyValueType
{
public:
  bool operator==(const TriggerPropertyDummyValueType&) const
  {
    return true;
  }
  bool operator!=(const TriggerPropertyDummyValueType&) const
  {
    return false;
  }
}
;
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

using variant_type = std::variant<bool,
                                  double,
                                  Color,
                                  int,
                                  AbstractPropertyOwner*,
                                  QString,
                                  std::size_t,
                                  TriggerPropertyDummyValueType,
                                  Vec2f,
                                  Vec2i,
                                  SplineType>;

template<typename T> T null_value();

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

constexpr Type get_variant_type(const std::string_view& v)
{
  return *std::find_if(variant_types.begin(), variant_types.end(), [v](const auto& type) {
    return variant_type_name(type) == v;
  });
}

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

//=== Channels
// Channels provide a unified way to read and write numerical values in a variant.

/**
 * @brief n_channels returns the number of channels in the type T.
 *  T == Color:            4 (hue, saturation, value, alpha)
 *  T == Vec2f, Vec2i:     2 (x, y)
 *  T == int, double bool: 1
 *  other:                 0
 */
template<typename T> std::size_t constexpr n_channels()
{
  if constexpr (std::is_same_v<T, double> || std::is_same_v<T, int> || std::is_same_v<T, bool>) {
    return 1;
  } else if constexpr (std::is_same_v<T, omm::Color>) {
    return 4;
  } else if constexpr (std::is_same_v<T, omm::Vec2f> || std::is_same_v<T, omm::Vec2i>) {
    return 2;
  } else {
    return 0;
  }
}

/**
 * @returns the numeric value of the given T at given channel
 * @note channel must not exceed the number of supported channels for a given type T.
 */
template<typename T> double get_channel_value(const T& v, [[maybe_unused]] std::size_t channel)
{
  if constexpr (std::is_same_v<T, double> || std::is_same_v<T, int>) {
    assert(channel == 0);
    return static_cast<double>(v);
  } else if constexpr (std::is_same_v<T, omm::Color>) {
    Color::Model model = v.model();
    if (model == Color::Named) {
      model = Color::Model::RGBA;
    }
    return v.components(model)[channel];
  } else if constexpr (std::is_same_v<T, omm::Vec2f> || std::is_same_v<T, omm::Vec2i>) {
    return static_cast<double>(v[channel]);
  } else if constexpr (std::is_same_v<T, bool>) {
    return v ? 1.0 : 0.0;
  } else {
    LFATAL("tried get numeric value from non-numeric type.");
    Q_UNREACHABLE();
    return 0.0;
  }
}

/**
 * @brief sets the channel of the given value to value.
 * @note channel must not exceed the number of supported channels for a given type T.
 */
template<typename T>
void set_channel_value(T& v, [[maybe_unused]] std::size_t channel, [[maybe_unused]] double value)
{
  if constexpr (std::is_same_v<T, double> || std::is_same_v<T, int>) {
    assert(channel == 0);
    v = value;
  } else if constexpr (std::is_same_v<T, omm::Color>) {
    const auto model = v.model();
    auto components = v.components(model);
    components[channel] = value;
    v.set_components(model, components);
  } else if constexpr (std::is_same_v<T, omm::Vec2f> || std::is_same_v<T, omm::Vec2i>) {
    v[channel] = value;
  } else if constexpr (std::is_same_v<T, bool>) {
    static constexpr double BOOL_THRESHOLD = 0.5;
    v = std::abs(value) > BOOL_THRESHOLD;
  } else {
    LFATAL("tried set numeric value of non-numeric type.");
    Q_UNREACHABLE();
  }
}

/**
 * @brief n_channels returns the number of supported channels for the type hold by the given
 *  concrete variant.
 */
std::size_t n_channels(const variant_type& variant);

/**
 * @brief get_channel_value return the value of the given channel of the type stored in the given
 *  variant. @code channel must not exceed the number of supported channels for the currently stored
 *  variant type.
 */
double get_channel_value(const variant_type& variant, std::size_t channel);

/**
 * @brief set_channel_value set the value of the given channel of the type stored in the given
 *  variant. @code channel must not exceed the number of supported channels for the currently stored
 *  variant type.
 */
void set_channel_value(variant_type& variant, std::size_t channel, double value);

}  // namespace omm
