#include "geometry/util.h"

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
