#pragma once

#include <QString>
#include <list>
#include <deque>

namespace omm
{

class Point;
class Edge;

class Face
{
public:
  ~Face();
  bool add_edge(const Edge& edge);
  [[nodiscard]] std::list<Point> points() const;
  [[nodiscard]] const std::deque<Edge>& edges() const;
  [[nodiscard]] double compute_aabb_area() const;
  [[nodiscard]] QString to_string() const;
private:
  std::deque<Edge> m_edges;
};

}  // namespace omm
