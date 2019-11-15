#include "variant.h"
#include "common.h"

namespace omm
{

std::size_t n_channels(const variant_type& variant)
{
  return std::visit([](auto&& v) { return n_channels<std::decay_t<decltype(v)>>(); }, variant);
}

double get_channel_value(const variant_type& variant, std::size_t channel)
{
  return std::visit([channel](auto&& v) {
    return get_channel_value<std::decay_t<decltype(v)>>(v, channel);
  }, variant);
}

void set_channel_value(variant_type& variant, std::size_t channel, const double value)
{
  std::visit([channel, value](auto&& v) {
    return set_channel_value<std::decay_t<decltype(v)>>(v, channel, value);
  }, variant);
}

std::ostream& operator<<(std::ostream& ostream, const TriggerPropertyDummyValueType&)
{
  ostream << "[TriggerPropertyDummyValueType]";
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const variant_type& v)
{
  print_variant_value(ostream, v);
  return ostream;
}

}  // namespace omm
