#pragma once

#include <armadillo>

namespace omm
{

class Color : private arma::vec4
{
public:
  Color(double r, double g, double b, double a = 1.0);
  Color(const arma::vec3& vec3, double a = 1.0);
  Color(const arma::vec4& vec4);

  Color operator%(const Color& other) const;
  Color operator+(const Color& other) const;
  Color operator*(double other) const;

  Color clamped() const;
  double& red() { return (*this)[0]; }
  double& green() { return (*this)[1]; }
  double& blue() { return (*this)[2]; }
  double& alpha() { return (*this)[3]; }
  double red() const { return (*this)[0]; }
  double green() const { return (*this)[1]; }
  double blue() const { return (*this)[2]; }
  double alpha() const { return (*this)[3]; }

  static Color RED;
  static Color GREEN;
  static Color BLUE;
  static Color YELLOW;
  static Color BLACK;
  static Color WHITE;

private:
  using base_type = arma::vec4;
  friend bool operator==(const Color& a, const Color& b);
  friend bool operator<(const Color& a, const Color& b);
};

bool operator==(const Color& a, const Color& b);
bool operator!=(const Color& a, const Color& b);
bool operator<(const Color& a, const Color& b);
std::ostream& operator<<(std::ostream& ostream, const Color& color);

}  // namespace
