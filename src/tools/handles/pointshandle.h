#pragma once

#include <armadillo>
#include "tools/handles/handle.h"

namespace omm
{

class PointsHandle : public Handle
{
public:
  explicit PointsHandle(std::vector<Point*>& point);
  bool contains(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;

private:
  std::vector<Point*> m_points;
};

}  // namespace omm
