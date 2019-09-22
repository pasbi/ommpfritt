#pragma once

#include <string>

namespace omm
{

class Typed
{
protected:
  Typed() = default;
public:
  virtual ~Typed() = default;
  virtual std::string type() const = 0;
};

}  // namespace omm
