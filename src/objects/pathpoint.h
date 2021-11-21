#pragma once

#include "geometry/point.h"

namespace omm
{

class Path;
class Segment;

class PathPoint
{
public:
  explicit PathPoint(const Point& geometry, Segment& segment);
  explicit PathPoint(Segment& segment);
  void set_geometry(const Point& point);
  [[nodiscard]] const Point& geometry() const;
  PathPoint copy(Segment& segment) const;
  [[nodiscard]] Segment& segment() const;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("PathPoint", "PathPoint");

  void set_selected(bool is_selected, bool update_buddies = true);
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

  [[nodiscard]] std::set<PathPoint*> joined_points() const;
  void join(std::set<PathPoint*> buddies);
  void disjoin();
  [[nodiscard]] Path* path() const;
  [[nodiscard]] Point compute_joined_point_geometry(PathPoint& joined) const;

  /**
   * @brief index returns the index of the point in the path.
   *  It can be used to persistently identify points for serialization.
   *  The index may change when the path is modified.
   */
  [[nodiscard]] std::size_t index() const;

private:
  Point m_geometry;
  Segment& m_segment;
  bool m_is_selected = false;
};

}  // namespace omm
