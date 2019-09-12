#pragma once

#include <map>
#include "animation/abstractfcurve.h"

namespace omm
{

class AbstractPropertyOwner;
class Property;

template<typename PropertyT>
class FCurve : public AbstractFCurve
{
public:
  using property_type = PropertyT;
  using value_type = typename PropertyT::value_type;
  enum class InterpolationMode { Step, Linear, Bezier };

  explicit FCurve() = default;
  virtual value_type interpolate(int frame) const = 0;

};

void register_fcurves();

}  // namespace omm
