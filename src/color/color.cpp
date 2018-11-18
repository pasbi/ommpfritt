#include <glog/logging.h>

#include "color/color.h"

namespace omm
{

Color::Color(double r, double g, double b, double a)
  : arma::vec4 { r, g, b, a }
{
}

Color Color::shaded(const double amount) const
{
  return *this % arma::vec4 { amount, amount, amount, 1.0 };
}

Color Color::clamped() const
{
  return arma::clamp(*this, 0, 1);
}

Color Color::RED = Color(1.0, 0.0, 0.0, 1.0);
Color Color::GREEN = Color(0.0, 1.0, 0.0, 1.0);
Color Color::BLUE = Color(0.0, 0.0, 1.0, 1.0);
Color Color::YELLOW = Color(0.0, 1.0, 1.0, 1.0);
Color Color::BLACK = Color(0.0, 0.0, 0.0, 1.0);
Color Color::WHITE = Color(1.0, 1.0, 1.0, 1.0);
}  // namespace omm
