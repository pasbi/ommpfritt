#include "color/color.h"

#include <cassert>
#include <algorithm>
#include <cmath>
#include <QtGlobal>
#include "logging.h"

namespace omm
{

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

void Color::to_hsv(double &hue, double &saturation, double &value) const
{
  std::array<double, 3> rgb;
  for (std::size_t i = 0; i < 3; ++i) {
    rgb[i] = std::clamp(m_components[i], 0.0, 1.0);
  }

  const double cmax = std::max(rgb[0], std::max(rgb[1], rgb[2]));
  const double cmin = std::min(rgb[0], std::min(rgb[1], rgb[2]));
  const double delta = cmax - cmin;

  if (delta == 0.0) {
    hue = 0.0;
  } else if (cmax == rgb[0]) {
    hue = std::fmod((rgb[1] - rgb[2])/delta, 6.0);
  } else if (cmax == rgb[1]) {
    hue = ((rgb[2] - rgb[0])/delta + 2.0);
  } else if (cmax == rgb[2]) {
    hue = ((rgb[0] - rgb[1])/delta + 4.0);
  } else {
    Q_UNREACHABLE();
  }
  hue *= M_PI/3.0;
  if (hue < 0.0) {
    hue += 2*M_PI;
  }

  saturation = cmax == 0.0 ? 0.0 : delta / cmax;
  value = cmax;
}

Color Color::from_hsv(double hue, double saturation, double value, double alpha)
{
  hue = std::fmod(hue, 2*M_PI);
  if (hue < 0.0) {
    hue += 2*M_PI;
  }
  const double h = hue * 3.0 / M_PI;
  const double c = value * saturation;
  const double x = c * (1.0 - std::abs(std::fmod(h, 2.0) - 1));
  const double m = value - c;
  const std::array<double, 3> rgb = [c, x, h]() -> std::array<double, 3> {
    switch (static_cast<int>(h)) {
    case 0:
      return {c, x, 0.0};
    case 1:
      return {x, c, 0.0};
    case 2:
      return {0.0, c, x};
    case 3:
      return {0.0, x, c};
    case 4:
      return {x, 0.0, c};
    case 5:
      return {c, 0.0, x};
    case 6:
    default:
      Q_UNREACHABLE();
      return {0.0, 0.0, 0.0};
    }
  }();

  return Color(rgb[0] + m, rgb[1] + m, rgb[2] + m, alpha);
}

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
