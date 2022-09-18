#include "path/edge.h"
#include "path/pathpoint.h"
#include "geometry/point.h"

namespace omm
{

Edge::Edge(std::shared_ptr<PathPoint> a, std::shared_ptr<PathPoint> b, Path* path)
    : m_path(path), m_a(a), m_b(b)
{
}

QString Edge::label() const
{
  static constexpr bool print_pointer = true;
  if constexpr (print_pointer) {
    return QString{"%1--%2 (this: %3, path: %4)"}.arg(m_a->debug_id(),
                                                      m_b->debug_id(),
                                                      QString::asprintf("%p", this),
                                                      QString::asprintf("%p", m_path));
  } else {
    return QString{"%1--%2"}.arg(m_a->debug_id(), m_b->debug_id());
  }
}

void Edge::flip() noexcept
{
  std::swap(m_a, m_b);
}

bool Edge::has_point(const PathPoint* p) noexcept
{
  return p == m_a.get() || p == m_b.get();
}

const std::shared_ptr<PathPoint>& Edge::a() const noexcept
{
  return m_a;
}

const std::shared_ptr<PathPoint>& Edge::b() const noexcept
{
  return m_b;
}

std::shared_ptr<PathPoint>& Edge::a() noexcept
{
  return m_a;
}

std::shared_ptr<PathPoint>& Edge::b() noexcept
{
  return m_b;
}

Path* Edge::path() const
{
  return m_path;
}

bool Edge::is_valid() const noexcept
{
  return m_a && m_b && m_a->path_vector() == m_b->path_vector();
}

bool Edge::contains(const PathPoint* p) const noexcept
{
  return m_a.get() == p || m_b.get() == p;
}

}  // namespace omm
