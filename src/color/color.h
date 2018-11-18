#pragma once

#include <armadillo>

namespace omm
{

class Color : public arma::vec4
{
public:
  explicit Color(double r, double g, double b, double a = 1.0);
  using arma::vec4::vec4;
  Color shaded(const double amount) const;
  Color clamped() const;

  static Color RED;
  static Color GREEN;
  static Color BLUE;
  static Color YELLOW;
  static Color BLACK;
  static Color WHITE;
};

}  // namespace
