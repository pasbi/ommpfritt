#pragma once

#include "geometry/boundingbox.h"
#include <armadillo>

namespace omm
{

class BoundingBox
{
public:
  BoundingBox(const std::vector<arma::vec2>& points);

  BoundingBox merge(const BoundingBox& other) const;
  BoundingBox intersect(const BoundingBox& other) const;
  bool contains(const BoundingBox& other) const;
  bool contains(const arma::vec2& point) const;
  bool is_empty() const;

  arma::vec2 top_left() const;
  arma::vec2 top_right() const;
  arma::vec2 bottom_left() const;
  arma::vec2 bottom_right() const;
  double left() const;
  double top() const;
  double right() const;
  double bottom() const;
  double width() const;
  double height() const;

private:
  arma::vec2 m_top_left;
  arma::vec2 m_bottom_right;
};

}  // namespace omm
