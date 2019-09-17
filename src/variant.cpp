#include "variant.h"
#include "common.h"

namespace omm
{

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
