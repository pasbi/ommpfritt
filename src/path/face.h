#pragma once

#include <QString>
#include <list>
#include <deque>

namespace omm
{

class Point;
class PathPoint;
class Edge;

class Face
{
public:
  Face() = default;
  ~Face();
  Face(const Face&) = default;
  Face(Face&&) = default;
  Face& operator=(const Face&) = default;
  Face& operator=(Face&&) = default;

  bool add_edge(const Edge& edge);

  /**
   * @brief points returns the geometry of each point around the face with proper tangents.
   * @note a face with `n` edges yields `n+1` points, because start and end point are listed
   *  separately.
   *  that's quite convenient for drawing paths.
   * @see path_points
   */
  [[nodiscard]] std::list<Point> points() const;

  /**
   * @brief path_points returns the points around the face.
   * @note a face with `n` edges yields `n` points, because start and end point are not listed
   *  separately.
   *  That's quite convenient for checking face equality.
   * @see points
   */
  [[nodiscard]] std::deque<PathPoint*> path_points() const;
  [[nodiscard]] const std::deque<Edge>& edges() const;
  [[nodiscard]] double compute_aabb_area() const;
  [[nodiscard]] QString to_string() const;

  friend bool operator==(const Face& a, const Face& b);

private:
  std::deque<Edge> m_edges;
};

}  // namespace omm
