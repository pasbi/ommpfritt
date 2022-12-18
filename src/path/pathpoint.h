#pragma once

#include "common.h"
#include "geometry/point.h"
#include <set>

namespace omm
{

class Path;  // NOLINT(bugprone-forward-declaration-namespace)
class PathVector;  // NOLINT(bugprone-forward-declaration-namespace)
class Edge;

class PathPoint
{
public:
  explicit PathPoint(const Point& geometry, const PathVector* path_vector);
  void set_geometry(const Point& point);
  [[nodiscard]] const Point& geometry() const;
  [[nodiscard]] Point& geometry();
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("PathPoint", "PathPoint");

  void set_selected(bool is_selected);
  [[nodiscard]] bool is_selected() const;

  // The PathPoint is identified by it's memory address, which hence must not change during its
  // lifetime.
  // Therefore we forbid movement and assignment, you will most presumably screw things otherwise.
  // Tip: encapsulate PathPoint in a unique_ptr and use references or pointers.
  PathPoint(const PathPoint& other) = delete;
  PathPoint(PathPoint&& other) = delete;
  PathPoint& operator=(const PathPoint& other) = delete;
  PathPoint& operator=(PathPoint&& other) = delete;
  ~PathPoint() = default;
  Point set_interpolation(InterpolationMode mode) const;

  [[nodiscard]] const PathVector* path_vector() const;

  /**
   * @brief debug_id returns an string to identify the point uniquely at this point in time
   * @note this id is not persistent.
   *  If a point is added or removed from its path, the id may change.
   */
  [[nodiscard]] QString debug_id() const;

  /**
   * @brief index returns the index of the point in the path.
   *  It can be used to persistently identify points for serialization.
   *  The index may change when the path is modified.
   */
  [[nodiscard]] std::size_t index() const;

  /**
   * @brief edges enumerates all edges that touch this point.
   */
  std::set<Edge*> edges() const;

private:
  const PathVector* m_path_vector;
  Point m_geometry;
  bool m_is_selected = false;
};

}  // namespace omm
