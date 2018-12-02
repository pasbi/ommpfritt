#include <glog/logging.h>

#include "color/color.h"

namespace omm
{

Color::Color(const arma::vec4& vec4)
  : arma::vec4(vec4)
{
}

Color::Color(double r, double g, double b, double a)
  : Color(arma::vec4 { r, g, b, a })
{
}

Color::Color(const arma::vec3& vec3, double a)
  : Color(vec3[0], vec3[1], vec3[2], a)
{
}

Color Color::clamped() const
{
  return base_type(arma::clamp(static_cast<base_type>(*this), 0, 1));
}


Color Color::operator%(const Color& other) const
{
  return base_type(static_cast<base_type>(*this) % static_cast<base_type>(other));
}

Color Color::operator+(const Color& other) const
{
  return base_type(static_cast<base_type>(*this) + static_cast<base_type>(other));
}

Color Color::operator*(double other) const
{
  return base_type(static_cast<base_type>(*this) * other);
}

Color Color::RED = Color(1.0, 0.0, 0.0, 1.0);
Color Color::GREEN = Color(0.0, 1.0, 0.0, 1.0);
Color Color::BLUE = Color(0.0, 0.0, 1.0, 1.0);
Color Color::YELLOW = Color(1.0, 1.0, 0.0, 1.0);
Color Color::BLACK = Color(0.0, 0.0, 0.0, 1.0);
Color Color::WHITE = Color(1.0, 1.0, 1.0, 1.0);

bool operator==(const Color& a, const Color& b)
{
  return arma::all(static_cast<Color::base_type>(a) == static_cast<Color::base_type>(b));
}

bool operator!=(const Color& a, const Color& b)
{
  return !(a == b);
}

}  // namespace omm
