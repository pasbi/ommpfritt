#include "python/pythonengine.h"
#include <armadillo>

namespace omm
{

arma::vec2 to_vec2(const pybind11::object& o);

}  // namespace omm
