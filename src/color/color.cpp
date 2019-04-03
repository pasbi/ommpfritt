#include "color/color.h"

#include <cassert>
#include <glog/logging.h>
#include <algorithm>

namespace omm
{

Color::Color(double r, double g, double b, double a) : m_components({r, g, b, a}) {}
Color::Color() : Color(0.0, 0.0, 0.0, 1.0) {}
Color::Color(const std::vector<double>& components)
{
  switch (components.size()) {
  case 4:
    m_components[3] = components[3];
    [[fallthrough]];
  case 3:
    m_components[0] = components[0];
    m_components[1] = components[1];
    m_components[2] = components[2];
  default:
    const auto msg = "Expected vector of size 3 or 4 but got " + std::to_string(components.size());
  }
}

Color Color::clamped() const
{
  Color clamped;
  for (std::size_t i = 0; i < m_components.size(); ++i) {
    clamped[i] = std::clamp((*this)[i], 0.0, 1.0);
  }
  return clamped;
}


double& Color::red() { return m_components[0]; }
double& Color::green() { return m_components[1]; }
double& Color::blue() { return m_components[2]; }
double& Color::alpha() { return m_components[3]; }
double& Color::operator[](const std::size_t i) { return m_components[i]; }
double Color::red() const { return m_components[0]; }
double Color::green() const { return m_components[1]; }
double Color::blue() const { return m_components[2]; }
double Color::alpha() const { return m_components[3]; }
double Color::operator[](const std::size_t i) const { return m_components[i]; }

Color Color::RED(1.0, 0.0, 0.0, 1.0);
Color Color::GREEN(0.0, 1.0, 0.0, 1.0);
Color Color::BLUE(0.0, 0.0, 1.0, 1.0);
Color Color::YELLOW(1.0, 1.0, 0.0, 1.0);
Color Color::BLACK(0.0, 0.0, 0.0, 1.0);
Color Color::WHITE(1.0, 1.0, 1.0, 1.0);

bool operator==(const Color& a, const Color& b)
{
  for (std::size_t i = 0; i < a.m_components.size(); ++i) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}

bool operator!=(const Color& a, const Color& b) { return !(a == b); }

bool operator<(const Color& a, const Color& b)
{
  for (size_t i = 0; i < a.m_components.size(); ++i) {
    if (a[i] < b[i]) {
      return true;
    } else if (a[i] > b[i]) {
      return false;
    }
  }
  return false;
}

std::ostream& operator<<(std::ostream& ostream, const Color& color)
{
  ostream << "Color[" << color.red() << ", " << color.green() << ", "
                      << color.blue() << ", " << color.alpha() << "]";
  return ostream;
}

Color operator*(const Color& c, const double s)
{
  Color result;
  for (std::size_t i = 0; i < 4; ++i) {
    result[i] = c[i] * s;
  }
  return result;
}

Color operator*(const double s, const Color& c)
{
  return c * s;
}

Color operator/(const Color& c, const double s)
{
  return c * (1.0/s);
}

}  // namespace omm
