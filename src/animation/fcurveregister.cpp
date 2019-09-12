#include "animation/floatfcurve.h"

namespace omm
{

void register_fcurves()
{
#define REGISTER_OBJECT(TYPE) AbstractFCurve::register_type<TYPE>(#TYPE)
  REGISTER_OBJECT(FloatFCurve);
#undef REGISTER_OBJECT
}

}  // namespace omm
