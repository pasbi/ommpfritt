#pragma once

#include <armadillo>
#include "tools/handles/handle.h"

namespace omm
{

class AxisHandle : public Handle
{
public:
  bool contains(const arma::vec2& point) const override;
  void set_direction(const arma::vec2& direction);

protected:
  arma::vec2 project_onto_axis(const arma::vec2& v) const;
  arma::vec2 m_direction;
};

}  // namespace omm
