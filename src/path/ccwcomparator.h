#pragma once

#include "path/dedge.h"

namespace omm
{

/**
 * @brief The CCWComparator class induces an order over edges starting at the end point of a common base.
 * Starting on the base edge, rotating around its end point counter clockwise, the `operator()`
 * returns whether its first argument is met first.
 * You can use it to decide along which edge (a or b) you want to travel if you want to go the
 * left- or right-most way, when coming via base.
 */
class CCWComparator
{
public:
  explicit CCWComparator(const omm::DEdge& base);
  [[nodiscard]] bool operator()(const omm::DEdge& a, const omm::DEdge& b) const noexcept;

private:
  omm::DEdge m_base;
  double m_base_arg;
};

}  // namespace omm
