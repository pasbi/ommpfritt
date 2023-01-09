#include "path/ccwcomparator.h"
#include "common.h"
#include "path/pathpoint.h"

namespace
{

double compare(const omm::DEdge& a, const omm::DEdge& b, const double base_arg) noexcept
{
  static constexpr auto eps = 0.001;
  assert(&a.start_point() == &b.start_point());
  const auto arg = [origin = a.start_point().geometry().position(), base_arg](const omm::DEdge& edge) {
    const auto eps_pos = omm::Vec2f(edge.to_geom_curve()->pointAt(eps)) - origin;
    return omm::python_like_mod(eps_pos.arg() - base_arg, 2.0 * M_PI);
  };

  return arg(a) < arg(b);
}

}  // namespace

namespace omm
{

CCWComparator::CCWComparator(const DEdge& base) : m_base(base), m_base_arg(base.end_angle())
{
}

bool CCWComparator::operator()(const DEdge& a, const DEdge& b) const noexcept
{
  return compare(a, b, m_base_arg);
}

}  // namespace omm
