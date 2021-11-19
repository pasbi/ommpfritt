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
  const Point& geometry() const;
  PathPoint copy(Segment& segment) const;
  Segment& segment() const;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("PathPoint", "PathPoint");

  void set_selected(bool is_selected);
  bool is_selected() const;

  // The PathPoint is identified by it's memory address, which hence must not change during its
  // lifetime.
  // Therefore we forbid movement and assignment, you will most presumably screw things otherwise.
  // Tip: encapsulate PathPoint in a unique_ptr and use references or pointers.
  PathPoint(const PathPoint& other) = delete;
  PathPoint(PathPoint&& other) = delete;
  PathPoint& operator=(const PathPoint& other) = delete;
  PathPoint& operator=(PathPoint&& other) = delete;

  std::set<PathPoint*> joined_points() const;
  void join(std::set<PathPoint*> buddies);
  void disjoin();
  Path* path() const;

  /**
   * @brief index returns the index of the point in the path.
   *  It can be used to persistently identify points for serialization.
   *  The index may change when the path is modified.
   */
  std::size_t index() const;

private:
  Point m_geometry;
  Segment& m_segment;
  bool m_is_selected = false;
  bool m_block_joined_points_update = false;
};

}  // namespace omm
