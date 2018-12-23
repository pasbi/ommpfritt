#pragma once

#include <vector>
#include <armadillo>

std::vector<double> to_stdvec(const arma::vec2& vec2);
arma::vec2 to_vec2(const std::vector<double>& stdvec);
