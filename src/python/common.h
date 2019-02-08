#pragma once

#include "python/pythonengine.h"
#include <armadillo>

namespace omm
{

template<typename VecT> VecT to_vec2(const pybind11::object& o)
{
  using value_type = typename VecT::value_type;
  const std::vector<value_type> std_vector = o.cast<std::vector<value_type>>();
  if (std_vector.size() != 2) {
    throw std::runtime_error("vector must have length of two.");
  }
  return VecT { std_vector[0], std_vector[1] };
}

template<typename VecT> pybind11::object from_vec2(const VecT& vec)
{
  using value_type = typename VecT::value_type;
  return pybind11::cast(arma::conv_to<std::vector<value_type>>::from(vec));
}

}  // namespace omm
