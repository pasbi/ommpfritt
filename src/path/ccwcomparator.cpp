#include "path/ccwcomparator.h"
#include "common.h"

namespace
{

template<std::size_t degree> [[nodiscard]] double start_angle(const omm::DEdge& edge) noexcept
{
  if constexpr (degree == 0) {
    return edge.start_angle();
  }
  return 0.0;
}

template<std::size_t degree> [[nodiscard]] double angle_to(const omm::DEdge& edge, const double base_arg) noexcept
{
  return omm::python_like_mod(::start_angle<degree>(edge) - base_arg, 2 * M_PI);
}

template<std::size_t max_degree, std::size_t degree = 0>
double compare(const omm::DEdge& a, const omm::DEdge& b, const double base_arg) noexcept
{
  const auto arg_a = ::angle_to<degree>(a, base_arg);
  const auto arg_b = ::angle_to<degree>(b, base_arg);
  static constexpr auto eps = 0.01 * M_PI / 180.0;
  if (std::abs(arg_a - arg_b) >= eps) {
    // a and b differ on this degree, the next degrees don't matter.
    return arg_a < arg_b;
  }
  static_assert(degree <= max_degree);
  if constexpr (degree == max_degree) {
    return false;  // a and b are identical in all degrees.
  } else {
    // a and b are identical on this degree, but may differ on the next.
    return ::compare<max_degree, degree + 1>(a, b, base_arg);
  }
}

}  // namespace

namespace omm
{

CCWComparator::CCWComparator(const DEdge& base) : m_base(base), m_base_arg(base.end_angle())
{
}

bool CCWComparator::operator()(const DEdge& a, const DEdge& b) const noexcept
{
  static constexpr auto max_degree = 1;  // TODO
  return compare<max_degree>(a, b, m_base_arg);
}

}  // namespace omm
