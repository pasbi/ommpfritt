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
  Typed(Typed&&) = default;
  Typed(const Typed&) = default;
  Typed& operator=(Typed&&) = default;
  Typed& operator=(const Typed&) = default;
  [[nodiscard]] virtual QString type() const = 0;
};

}  // namespace omm
