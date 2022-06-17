#pragma once

#include "path/edge.h"
#include <QString>
#include <list>
#include <deque>
#include "geometry/vec2.h"

class QPainterPath;

namespace omm
{

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

class Point;
class PathPoint;

class Face
{
public:
  Face() = default;
  ~Face();
  Face(const Face&) = default;
  Face(std::deque<Edge> edges);
  Face(Face&&) = default;
  Face& operator=(const Face&) = default;
  Face& operator=(Face&&) = default;

  bool add_edge(const Edge& edge);
  [[nodiscard]] QPainterPath to_painter_path() const;

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
  [[nodiscard]] bool is_valid() const;

  [[nodiscard]] bool contains(const Face& other) const;
  [[nodiscard]] bool contains(const Vec2f& pos) const;

  [[nodiscard]] bool operator==(const Face& other) const;
  [[nodiscard]] bool operator!=(const Face& other) const;
  [[nodiscard]] bool operator<(const Face& other) const;

  class ReferencePolisher;
  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);

private:
  std::deque<Edge> m_edges;
};

}  // namespace omm
