#include "geometry/util.h"
#include <cmath>

std::vector<double> to_stdvec(const arma::vec2& vec2)
{
  return arma::conv_to<std::vector<double>>::from(vec2);
}

arma::vec2 to_vec2(const std::vector<double>& stdvec)
{
  if (stdvec.size() != 2) {
    const auto msg = "Expected vector of size 2 but got " + std::to_string(stdvec.size());
    throw std::length_error(msg);
  }
  return arma::vec2 { stdvec[0], stdvec[1] };
}

arma::vec2 get_scale(const arma::vec2& pos, const arma::vec2& delta, const arma::vec2& direction)
{
  const auto factor = [](const double new_pos, const double delta, const double constraint) {
    const double old_pos = new_pos - delta;
    const double epsilon = 0.0001;
    if (std::abs(old_pos) < epsilon) {
      return 1.0;
    } else {
      double s = new_pos / old_pos;
      s = std::copysign(std::pow(std::abs(s), std::abs(constraint)), s);
      return s;
      if (std::abs(s) < 0.1) {
        return std::copysign(1.0, s);
      } else {
        return s;
      }
    }
  };
  const arma::vec old_pos = pos - delta;
  const arma::vec d = direction / arma::norm(direction);
  return arma::vec({ factor(pos(0), delta(0), d(0)), factor(pos(1), delta(1), d(1)) });
}
