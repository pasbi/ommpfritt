#pragma once

#include <vector>
#include <armadillo>

std::vector<double> to_stdvec(const arma::vec2& vec2);
arma::vec2 to_vec2(const std::vector<double>& stdvec);
arma::vec2 get_scale(const arma::vec2& pos, const arma::vec2& delta, const arma::vec2& direction);
