#pragma once

#include <iterator>
#include <array>
#include <vector>
#include <QColor>

namespace omm
{

class Color
{
public:
  inline explicit constexpr Color(double r, double g, double b, double a = 1.0)
    : m_components({r, g, b, a}) {}
  explicit Color(const std::vector<double>& components);
  explicit Color(const std::array<double, 4>& rgba);
  explicit Color(const std::array<double, 3>& rgb);
  Color(const QColor& c);
  explicit Color();
  Color(const std::string& code);

  enum class BlendMode {};
  Color blend(const Color& other, BlendMode mode) const;

  Color clamped() const;
  double& red();
  double& green();
  double& blue();
  double& alpha();
  double& operator[](const std::size_t i);
  double red() const;
  double green() const;
  double blue() const;
  double alpha() const;
  double operator[](const std::size_t i) const;
  void to_hsv(double& hue, double& saturation, double& value) const;
  static Color from_hsv(double hue, double saturation, double value, double alpha = 1.0);
  operator QColor() const;
  bool decode(const std::string& code);
  std::string to_hex(bool no_alpha = false) const;
  Color& operator+=(const Color& a);
  Color& operator+=(double a);
  Color& operator-=(const Color& a);
  Color& operator-=(double a);
  Color& operator*=(const Color& a);
  Color& operator*=(double a);
  Color& operator/=(const Color& a);
  Color& operator/=(double a);
  Color operator-() const;

private:
  std::array<double, 4> m_components;
  friend bool operator==(const Color& a, const Color& b);
  friend bool operator<(const Color& a, const Color& b);
};

namespace Colors
{

static constexpr Color RED = Color(1.0, 0.0, 0.0, 1.0);
static constexpr Color GREEN = Color(0.0, 1.0, 0.0, 1.0);
static constexpr Color BLUE = Color(0.0, 0.0, 1.0, 1.0);
static constexpr Color YELLOW = Color(1.0, 1.0, 0.0, 1.0);
static constexpr Color BLACK = Color(0.0, 0.0, 0.0, 1.0);
static constexpr Color WHITE = Color(1.0, 1.0, 1.0, 1.0);

}

bool operator==(const Color& a, const Color& b);
bool operator!=(const Color& a, const Color& b);
bool operator<(const Color& a, const Color& b);
std::ostream& operator<<(std::ostream& ostream, const Color& color);

Color operator*(const Color& c, const double s);
Color operator*(const double s, const Color& c);
Color operator*(const Color& a, const Color& b);
Color operator/(const Color& c, const double s);
Color operator/(const double s, const Color& c);
Color operator/(const Color& a, const Color& b);
Color operator+(const Color& c, const double s);
Color operator+(const double s, const Color& c);
Color operator+(const Color& a, const Color& b);
Color operator-(const Color& c, const double s);
Color operator-(const double s, const Color& c);
Color operator-(const Color& a, const Color& b);



}  // namespace
