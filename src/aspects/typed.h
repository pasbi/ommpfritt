#pragma once

#include <QString>

namespace omm
{
class Typed
{
protected:
  Typed() = default;

public:
  virtual ~Typed() = default;
  [[nodiscard]] virtual QString type() const = 0;
};

}  // namespace omm
