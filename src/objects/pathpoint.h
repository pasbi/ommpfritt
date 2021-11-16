#pragma once

#include "geometry/point.h"

namespace omm
{

class Segment;

class PathPoint
{
public:
  explicit PathPoint(const Point& geometry, Segment* segment = nullptr);
  explicit PathPoint(Segment* segment = nullptr);
  void set_geometry(const Point& point);
  const Point& geometry() const;
  Point& geometry();
  PathPoint copy(Segment& segment) const;
  Segment& segment() const;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("PathPoint", "PathPoint");

  void set_selected(bool is_selected);
  bool is_selected() const;
  friend void swap(PathPoint& a, PathPoint& b);

  // The PathPoint is identified by it's memory address, which hence must not change during its
  // lifetime.
  // Therefore we forbid movement and assignment, you will most presumably screw things otherwise.
  // Tip: encapsulate PathPoint in a unique_ptr and use references or pointers.
  PathPoint(const PathPoint& other) = delete;
  PathPoint(PathPoint&& other) = delete;
  PathPoint& operator=(const PathPoint& other) = delete;
  PathPoint& operator=(PathPoint&& other) = delete;

private:
  Point m_geometry;
  Segment* m_segment;
  bool m_is_selected = false;
};

}  // namespace omm
