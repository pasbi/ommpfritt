#include "variant.h"
#include "common.h"
#include "aspects/abstractpropertyowner.h"

namespace omm
{
std::size_t n_channels(const variant_type& variant)
{
  return std::visit([](auto&& v) { return n_channels<std::decay_t<decltype(v)>>(); }, variant);
}

double get_channel_value(const variant_type& variant, std::size_t channel)
{
  return std::visit(
      [channel](auto&& v) { return get_channel_value<std::decay_t<decltype(v)>>(v, channel); },
      variant);
}

void set_channel_value(variant_type& variant, std::size_t channel, const double value)
{
  std::visit(
      [channel, value](auto&& v) {
        return set_channel_value<std::decay_t<decltype(v)>>(v, channel, value);
      },
      variant);
}

QString to_string(const variant_type& var)
{
  const auto value_string = std::visit([](const auto& v) {
    using T = std::decay_t<decltype(v)>;
    if constexpr (typelist_contains<T, bool, int, double, QString, std::size_t>()) {
      return QString{"%1"}.arg(v);
    } else if constexpr (typelist_contains<T, Color, AbstractPropertyOwner*, SplineType, Vec2i, Vec2f>()) {
      if constexpr (std::is_pointer_v<T>) {
        return v->to_string();
      } else {
        return v.to_string();
      }
    } else {
      return QString{};
    }
  }, var);

  const auto type_string = std::visit([](const auto& var) {
    return variant_type_name(get_variant_type<std::decay_t<decltype(var)>>());
  }, var);
  return QObject::tr("%1[%2]").arg(type_string.data(), value_string);
}

template<> bool null_value<bool>()
{
  return false;
}
template<> double null_value<double>()
{
  return 0.0;
}
template<> Color null_value<Color>()
{
  return Color(Color::Model::RGBA, {0.0, 0.0, 0.0, 0.0});
}
template<> int null_value<int>()
{
  return 0;
}
template<> Vec2f null_value<Vec2f>()
{
  return {0.0, 0.0};
}
template<> Vec2i null_value<Vec2i>()
{
  return {0, 0};
}
template<> SplineType null_value<SplineType>()
{
  return SplineType();
}

// This should be constexpr.
// however, currently used libs do not have constexpr `std::find_if`.
Type get_variant_type(const std::string_view& v)
{
  return *std::find_if(variant_types.begin(), variant_types.end(), [v](const auto& type) {
    return variant_type_name(type) == v;
  });
}

}  // namespace omm
