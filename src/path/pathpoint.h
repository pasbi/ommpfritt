#pragma once

#include "geometry/point.h"
#include "transparentset.h"
#include <set>

namespace omm
{

// NOLINTNEXTLINE(bugprone-forward-declaration-namespace)
class Path;

// NOLINTNEXTLINE(bugprone-forward-declaration-namespace)
class PathVector;

class PathPoint
{
public:
  explicit PathPoint(const Point& geometry, Path& path);
  explicit PathPoint(Path& path);
  void set_geometry(const Point& point);
  [[nodiscard]] const Point& geometry() const;
  PathPoint copy(Path& path) const;
  [[nodiscard]] Path& path() const;
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

  [[nodiscard]] ::transparent_set<PathPoint*> joined_points() const;
  void join(::transparent_set<PathPoint*> buddies);
  void disjoin();
  [[nodiscard]] PathVector* path_vector() const;
  [[nodiscard]] Point compute_joined_point_geometry(PathPoint& joined) const;
  [[nodiscard]] bool is_dangling() const;

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

private:
  Point m_geometry;
  Path& m_path;
  bool m_is_selected = false;
};

}  // namespace omm
