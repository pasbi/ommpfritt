#pragma once

#include <armadillo>
#include "external/json_fwd.hpp"

namespace omm
{

class ObjectTransformation : public arma::Mat<double>::fixed<3, 3>
{
public:
  static constexpr auto N_ROWS = 3;
  static constexpr auto N_COLS = 3;
  using arma::Mat<double>::fixed<N_ROWS, N_COLS>::fixed;
};

std::ostringstream& operator<<(std::ostringstream& ostream, const ObjectTransformation& t);

}  // namespace omm