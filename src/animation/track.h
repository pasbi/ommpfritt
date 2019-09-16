#pragma once

#include <map>
#include "animation/abstracttrack.h"

namespace omm
{

class AbstractPropertyOwner;
class Property;

template<typename PropertyT> class Track : public AbstractTrack
{
public:
  using property_type = PropertyT;
  using value_type = typename PropertyT::value_type;
  enum class InterpolationMode { Step, Linear, Bezier };

  explicit Track() = default;
  virtual value_type interpolate(int frame) const = 0;

};

void register_fcurves();

}  // namespace omm
