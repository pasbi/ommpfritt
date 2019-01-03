#include "python/common.h"
#include "python/pythonengine.h"
#include <armadillo>
#include <pybind11/stl.h>
#include <utility>

namespace omm
{

arma::vec2 to_vec2(const pybind11::object& o)
{
  const std::vector<double> std_vector = o.cast<std::vector<double>>();
  if (std_vector.size() == 2) {
    return arma::vec2 { std_vector[0], std_vector[1] };
  } else {
    throw std::runtime_error("vector must have length of two.");
  }
}

}  // namespace omm
