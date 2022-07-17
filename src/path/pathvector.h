#pragma once

#include "geometry/vec2.h"
#include <deque>
#include <memory>
#include <set>

class QPainterPath;
class QPainter;

namespace omm
{

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

class EnhancedPathVector;

// NOLINTNEXTLINE(bugprone-forward-declaration-namespace)
class Path;
class PathPoint;
class PathObject;
class Scene;
class Face;

// NOLINTNEXTLINE(bugprone-forward-declaration-namespace)
class PathVector
{
public:
  PathVector(PathObject* path_object = nullptr);
  PathVector(const PathVector& other, PathObject* path_object = nullptr);
  PathVector(PathVector&& other) noexcept;
  PathVector& operator=(const PathVector& other);
  PathVector& operator=(PathVector&& other) noexcept;
  ~PathVector();
  friend void swap(PathVector& a, PathVector& b) noexcept;

  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);
  [[nodiscard]] PathPoint& point_at_index(std::size_t index) const;

  [[nodiscard]] QPainterPath to_painter_path() const;
  [[nodiscard]] std::set<Face> faces() const;
  [[nodiscard]] std::size_t point_count() const;
  [[nodiscard]] std::deque<Path*> paths() const;
  [[nodiscard]] Path* find_path(const PathPoint& point) const;
  Path& add_path(std::unique_ptr<Path> path);
  Path& add_path();
  std::unique_ptr<Path> remove_path(const Path& path);
  [[nodiscard]] std::shared_ptr<PathPoint> share(const PathPoint& path_point) const;
  [[nodiscard]] std::deque<PathPoint*> points() const;
  [[nodiscard]] std::deque<PathPoint*> selected_points() const;
  void deselect_all_points() const;
  [[nodiscard]] PathObject* path_object() const;
  void draw_point_ids(QPainter& painter) const;


  /**
   * @brief is_valid returns true if this path vector is valid.
   * A PathVector must be valid before and after every public member function.
   * Undefined Behaviour may occur, if a path vector is not valid.
   * This function is a very convenient debugging tool, it shouldn't be used otherwise.
   */
  [[nodiscard, maybe_unused]] bool is_valid() const;

private:
  PathObject* m_path_object = nullptr;
  std::deque<std::unique_ptr<Path>> m_paths;
};

}  // namespace omm
