#pragma once

#include "observed.h"

namespace omm
{

template<typename T> class Structure
{
public:
  using item_type = T;
  virtual ~Structure() {}
  virtual const T* predecessor(const T& item) const = 0;
};

}  // namespace omm
