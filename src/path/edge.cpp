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
  static constexpr auto p2s = [](const auto& p) {
    return QString::asprintf("%8p", static_cast<void*>(p.get()));
  };
  return QString{"%1--%3"}.arg(p2s(m_a), p2s(m_b));
}

void Edge::flip() noexcept
{
  std::swap(m_a, m_b);
}

bool Edge::has_point(const PathPoint* p) noexcept
{
  return p == m_a.get() || p == m_b.get();
}

std::shared_ptr<PathPoint> Edge::a() const noexcept
{
  return m_a;
}

std::shared_ptr<PathPoint> Edge::b() const noexcept
{
  return m_b;
}

bool Edge::operator<(const Edge& other) const noexcept
{
  static constexpr auto as_tuple = [](const Edge& e) {
    return std::tuple{e.a().get(), e.b().get()};
  };
  return as_tuple(*this) < as_tuple(other);
}

Path* Edge::path() const
{
  return m_path;
}

}  // namespace omm
