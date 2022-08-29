#include "path/pathpoint.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathvector.h"
#include "objects/pathobject.h"
#include "scene/scene.h"


namespace
{

std::pair<omm::PolarCoordinates, omm::PolarCoordinates>
compute_smooth_tangents(const omm::PathPoint& point, const omm::Path& path)
{
  const auto points = path.points();
  const auto it = std::find_if(points.begin(), points.end(), [&point](const auto* candidate) {
    return &point == candidate;
  });
  assert(it != points.end());
  const auto* const bwd = it == points.begin() ? nullptr : *prev(it);
  const auto* const fwd = next(it) == points.end() ? nullptr : *next(it);


  const auto pos = point.geometry().position();

  static constexpr auto t = 1.0 / 3.0;
  using PC = omm::PolarCoordinates;
  static constexpr auto lerp = [](const double t, const auto& a, const auto& b) {
    return (1.0 - t) * a + t * b;
  };

  auto bwd_pc = bwd == nullptr ? PC() : PC(lerp(t, pos, bwd->geometry().position()) - pos);
  auto fwd_pc = fwd == nullptr ? PC() : PC(lerp(t, pos, fwd->geometry().position()) - pos);

  if (bwd != nullptr && fwd != nullptr) {
    const auto p_bwd = bwd->geometry().position();
    const auto p_fwd = fwd->geometry().position();
    const auto p_bwd_reflected = (2.0 * pos - p_bwd);
    fwd_pc.argument = omm::PolarCoordinates(lerp(0.5, p_bwd_reflected, p_fwd) - pos).argument;
    bwd_pc.argument = (-fwd_pc).argument;
  }

  return {bwd_pc, fwd_pc};
}

}  // namespace

namespace omm
{

PathPoint::PathPoint(const Point& geometry, PathVector* path_vector)
  : m_path_vector(path_vector)
  , m_geometry(geometry)
{
}

Point PathPoint::set_interpolation(InterpolationMode mode) const
{
  auto copy = m_geometry;
  auto& tangents = copy.tangents();
  for (auto& [key, tangent] : tangents) {
    switch (mode)
    {
    case InterpolationMode::Linear:
      tangent.magnitude = 0.0;
      break;
    case InterpolationMode::Bezier:
      break;
    case InterpolationMode::Smooth:
      if (key.direction == Point::Direction::Forward) {
        const auto [bwd, fwd] = compute_smooth_tangents(*this, *key.path);
        tangents[{key.path, Point::Direction::Forward}] = fwd;
        tangents[{key.path, Point::Direction::Backward}] = bwd;
      }
      break;
    }
  }
  return copy;
}

PathVector* PathPoint::path_vector() const
{
  return m_path_vector;
}

QString PathPoint::debug_id() const
{
  static constexpr bool print_pointer = false;
  if constexpr (print_pointer) {
    return QString{"%1 (%2)"}.arg(index()).arg(QString::asprintf("%p", this));
  } else {
    return QString{"%1"}.arg(index());
  }
}

std::size_t PathPoint::index() const
{
  assert(path_vector() != nullptr);
  const auto points = path_vector()->points();
  return std::distance(points.begin(), std::find(points.begin(), points.end(), this));
}

std::set<Edge*> PathPoint::edges() const
{
  if (m_path_vector == nullptr) {
    return {};
  }
  auto edges = util::transform<std::set>(m_path_vector->edges());
  std::erase_if(edges, [this](const auto* edge) { return !edge->contains(this); });
  return edges;
}

void PathPoint::set_geometry(const Point& point)
{
  m_geometry = point;
}

Point& PathPoint::geometry()
{
  return m_geometry;
}

const Point& PathPoint::geometry() const
{
  return m_geometry;
}

bool PathPoint::is_selected() const
{
  return m_is_selected;
}

void PathPoint::set_selected(bool selected)
{
  m_is_selected = selected;
}

}  // namespace omm
