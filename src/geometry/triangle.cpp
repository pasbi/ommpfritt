#include "geometry/triangle.h"
#include "geometry/util.h"
#include "common.h"
#include "logging.h"

namespace omm
{

Triangle::Triangle(const std::array<Vec2f, 3>& points) : points(points) {}

double Triangle::area() const
{
  const auto& a = points[0];
  const auto& b = points[1];
  const auto& c = points[2];
  return 0.5 * (a.x-c.x)*(b.y-a.y) - (a.x-b.x)*(c.y-a.y);
}

bool Triangle::contains(const Vec2f &p) const
{
  for (auto&& [a, b] : edges()) {
    const auto s = (p.x - b.x) * (a.y - b.y) - (a.x - b.x) * (p.y - b.y);
    if (s < 0 || s > 1) {
      return false;
    }
  }
  return true;
}

bool Triangle::crosses_edge(const Edge& candidate, Edge& crossing) const
{
  for (const Edge& triangle_edge : edges()) {
    static constexpr auto eps = 10e-3;
    if (Edge::intersects(candidate, triangle_edge, eps)) {
      crossing = triangle_edge;
      return true;
    }
  }
  return false;
}

bool Triangle::is_neighbor(const Triangle &other) const
{
  auto n = 0;
  for (const auto& p : points) {
    if (other.has_vertex(p)) {
      n += 1;
    }
  }

  return n == 2;
}

bool Triangle::has_vertex(const Vec2f &p, std::size_t* index) const
{
  const auto it = std::find(points.begin(), points.end(), p);
  if (it == points.end()) {
    return false;
  } else {
    if (index != nullptr) {
      *index = static_cast<std::size_t>(std::distance(points.begin(), it));
    }
    return true;
  }
}

bool Triangle::has_edge(const Edge &edge) const
{
  return has_vertex(edge.a) && has_vertex(edge.b);
}

std::vector<Edge> Triangle::edges() const
{
  return {
    Edge(points[0], points[1]),
    Edge(points[1], points[2]),
    Edge(points[2], points[0]),
  };
}

void Triangle::swap_diagonal(Triangle &a, Triangle &b)
{
  assert(a.is_neighbor(b));
  assert(a != b);
  const auto other = [](const std::size_t& a, const std::size_t& b) -> std::size_t {
    switch (a+b) {
    case 1: return 2;   // 0 and 1
    case 2: return 1;   // 0 and 2
    case 3: return 0;   // 1 and 2
    default: Q_UNREACHABLE();
    }
  };
  Vec2f old_diagonal_start, old_diagonal_end;
  Vec2f new_dialonal_start, new_dialonal_end;
  bool initialized = false;
  const auto indices = std::vector<std::pair<std::size_t, std::size_t>> { {0, 1}, {0, 2}, {1, 2} };
  for (auto&& [i_a_1, i_a_2] : indices) {
    std::size_t i_b_1, i_b_2;
    if (b.has_vertex(a.points[i_a_1], &i_b_1) && b.has_vertex(a.points[i_a_2], &i_b_2)) {
      assert(initialized == false);  // otherwise, `a == b`
      old_diagonal_start = a.points[i_a_1];
      old_diagonal_end = a.points[i_a_2];
      new_dialonal_start = a.points[other(i_a_1, i_a_2)];
      new_dialonal_end = b.points[other(i_b_1, i_b_2)];
      initialized = true;
    }
  }
  assert(initialized == true);  // otherwise, `a` is not a neighbor of `b`

  a = Triangle({ old_diagonal_start, new_dialonal_start, new_dialonal_end });
  b = Triangle({ old_diagonal_start, new_dialonal_start, new_dialonal_end });


}

bool Triangle::operator==(const Triangle &other) const
{
  for (const auto& p : points) {
    if (!other.has_vertex(p)) {
      return false;
    }
  }
  return true;
}

bool Triangle::operator!=(const Triangle &other) const
{
  return !(*this == other);
}

std::ostream& operator <<(std::ostream& stream, const Triangle &triangle)
{
  stream << "Triangle";
  ::operator<<(stream, std::vector(triangle.points.begin(), triangle.points.end()));
  return stream;
}

}  // namespace omm
