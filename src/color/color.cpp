#include "color/color.h"

#include <cassert>
#include <algorithm>
#include <cmath>
#include <QtGlobal>
#include "logging.h"
#include <iomanip>
#include <iostream>

namespace
{

bool decode_hex(const std::string& code, omm::Color& color)
{
  if (code.at(0) != '#' || (code.size() != 7 && code.size() != 9)) {
    return false;
  } else {
    if (code.size() == 7 || code.size() == 9) {
      color.red()   = std::strtol(&code.substr(1, 2).at(0), nullptr, 16) / 255.0;
      color.green() = std::strtol(&code.substr(3, 2).at(0), nullptr, 16) / 255.0;
      color.blue()  = std::strtol(&code.substr(5, 2).at(0), nullptr, 16) / 255.0;
      if (code.size() == 9) {
        color.alpha() = std::strtol(&code.substr(7, 2).at(0), nullptr, 16) / 255.0;
      } else {
        color.alpha() = 1.0;
      }
      return true;
    } else {
      return false;
    }
  }
}

}  // namespace

namespace omm
{

Color::Color() : Color(0.0, 0.0, 0.0, 1.0) {}

Color::Color(const std::string& code)
{
  if (!decode(code)) {
    LWARNING << "Failed to decode color '" << code << "'";
  }
}

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

Color::Color(const std::array<double, 4>& rgba) : m_components(rgba)
{
}

Color::Color(const std::array<double, 3>& rgb) : m_components({ rgb[0], rgb[1], rgb[2], 1.0 })
{
}

Color::Color(const QColor& c) : Color(c.redF(), c.greenF(), c.blueF(), c.alphaF())
{
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
  hue /= 2 * M_PI;
}

Color Color::from_hsv(double hue, double saturation, double value, double alpha)
{
  hue *= 2.0 * M_PI;
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

bool Color::decode(const std::string& code)
{
  return decode_hex(code, *this);
}

std::string Color::to_hex(bool no_alpha) const
{
  static const auto to_hex = [](float f) {
    const int i = std::clamp(static_cast<int>(std::round(f*255)), 0, 255);
    std::ostringstream ostr;
    ostr << std::setw(2) << std::setfill('0') << std::hex << i;
    const std::string str = ostr.str();
    assert(str.size() == 2);
    return str;
  };
  return "#" + to_hex(red()) + to_hex(green()) + to_hex(blue()) + (no_alpha ? "" : to_hex(alpha()));
}

Color& Color::operator+=(const Color& a)
{
  *this = *this + a;
  return *this;
}

Color& Color::operator+=(double a)
{
  *this = *this + a;
  return *this;
}

Color& Color::operator-=(const Color& a)
{
  *this = *this - a;
  return *this;
}

Color& Color::operator-=(double a)
{
  *this = *this - a;
  return *this;
}

Color& Color::operator*=(const Color& a)
{
  *this = *this * a;
  return *this;
}

Color& Color::operator*=(double a)
{
  *this = *this * a;
  return *this;
}

Color& Color::operator/=(const Color& a)
{
  *this = *this / a;
  return *this;
}

Color& Color::operator/=(double a)
{
  *this = *this / a;
  return *this;
}

Color Color::operator-() const
{
  return Color(-red(), -green(), -blue(), -alpha());
}

QColor Color::to_qcolor() const
{
  QColor qc;
  qc.setRedF(red());
  qc.setGreenF(green());
  qc.setBlueF(blue());
  qc.setAlphaF(alpha());
  return qc;
}

Color Color::abs() const
{
  Color color = *this;
  for (double& c : color.m_components) {
    c = std::abs(c);
  }
  return color;
}

double Color::max_component() const
{
  double max = m_components[0];
  for (const double c : m_components) {
    max = std::max(c, max);
  }
  return max;
}

Color Color::set(Color::Role role, double value) const
{

  switch (role) {
  case Role::Red:
    [[fallthrough]];
  case Role::Blue:
    [[fallthrough]];
  case Role::Alpha:
    [[fallthrough]];
  case Role::Green:
  {
    Color color = *this;
    switch (role) {
    case Role::Red:
      color.red() = value;
      return color;
    case Role::Green:
      color.green() = value;
      return color;
    case Role::Alpha:
      color.alpha() = value;
      return color;
    case Role::Blue:
      color.blue() = value;
      return color;
    default:
      Q_UNREACHABLE();
      return Color();
    }
  }
  case Role::Hue:
    [[fallthrough]];
  case Role::Saturation:
    [[fallthrough]];
  case Role::Value:
  {
    double h, s, v;
    to_hsv(h, s, v);
    switch (role) {
    case Role::Hue:
      return Color::from_hsv(value, s, v, alpha());
    case Role::Saturation:
      return Color::from_hsv(h, value, v, alpha());
    case Role::Value:
      return Color::from_hsv(h, s, value, alpha());
    default:
      Q_UNREACHABLE();
      return Color();
    }
  }
  default:
    Q_UNREACHABLE();
    return Color();
  }
}

double Color::get(Color::Role role) const
{
  switch (role) {
  case Role::Red:
    return red();
  case Role::Blue:
    return blue();
  case Role::Alpha:
    return alpha();
  case Role::Green:
    return green();
  case Role::Hue:
    [[fallthrough]];
  case Role::Saturation:
    [[fallthrough]];
  case Role::Value:
  {
    double h, s, v;
    to_hsv(h, s, v);
    switch (role) {
    case Role::Hue:
      return h;
    case Role::Saturation:
      return s;
    case Role::Value:
      return v;
    default:
      Q_UNREACHABLE();
      return 0.0;
    }
  }
  default:
    Q_UNREACHABLE();
    return 0.0;
  }
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
  return c * Color(s, s, s, s);
}

Color operator*(const double s, const Color& c)
{
  return c * s;
}

Color operator*(const Color& a, const Color& b)
{
  return Color(a.red() * b.red(), a.green() * b.green(), a.blue() * b.blue(), a.alpha() * b.alpha());
}

Color operator/(const Color& c, const double s)
{
  return c / Color(s, s, s, s);
}

Color operator/(const double s, const Color& c)
{
  return Color(s, s, s, s) / c;
}

Color operator/(const Color& a, const Color& b)
{
  return Color(a.red() / b.red(), a.green() / b.green(), a.blue() / b.blue(), a.alpha() / b.alpha());
}

Color operator+(const Color& c, const double s)
{
  return c + Color(s, s, s, s);
}

Color operator+(const double s, const Color& c)
{
  return c + s;
}

Color operator+(const Color& a, const Color& b)
{
  return Color(a.red() + b.red(), a.green() + b.green(), a.blue() + b.blue(), a.alpha() + b.alpha());
}

Color operator-(const Color& c, const double s)
{
  return c + (-s);
}

Color operator-(const double s, const Color& c)
{
  return s + (-c);
}

Color operator-(const Color& a, const Color& b)
{
  return a + (-b);
}

}  // namespace omm
