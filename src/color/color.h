#pragma once

#include <iterator>
#include <array>
#include <vector>

namespace omm
{

class Color
{
public:
  explicit Color(double r, double g, double b, double a = 1.0);
  explicit Color(const std::vector<double>& components);
  explicit Color();

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

  static Color RED;
  static Color GREEN;
  static Color BLUE;
  static Color YELLOW;
  static Color BLACK;
  static Color WHITE;

private:
  std::array<double, 4> m_components;
  friend bool operator==(const Color& a, const Color& b);
  friend bool operator<(const Color& a, const Color& b);
};

bool operator==(const Color& a, const Color& b);
bool operator!=(const Color& a, const Color& b);
bool operator<(const Color& a, const Color& b);
std::ostream& operator<<(std::ostream& ostream, const Color& color);

Color operator*(const Color& c, const double s);
Color operator*(const double s, const Color& c);
Color operator/(const Color& c, const double s);

}  // namespace
