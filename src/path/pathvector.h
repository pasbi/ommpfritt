#pragma once

#include "geometry/vec2.h"
#include <deque>
#include <memory>

class QPainterPath;

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
class DisjointPathPointSetForest;
class Scene;

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

  /**
   * @brief share_joined_points use the `joined_points` forest to register joined points.
   *  The currently owned joined points will be added to the shared joined points.
   */
  std::unique_ptr<DisjointPathPointSetForest> share_joined_points(DisjointPathPointSetForest& joined_points);
  void unshare_joined_points(std::unique_ptr<DisjointPathPointSetForest> joined_points);

  /**
   * @brief join_points_shared returns true if the joined points are shared or false otherwise.
   */
  [[nodiscard]] bool joined_points_shared() const;

  static constexpr auto SEGMENTS_POINTER = "segments";
  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);
  [[nodiscard]] PathPoint& point_at_index(std::size_t index) const;

  [[nodiscard]] QPainterPath outline() const;
  [[nodiscard]] std::vector<QPainterPath> faces() const;
  [[nodiscard]] std::size_t point_count() const;
  [[nodiscard]] std::deque<Path*> paths() const;
  [[nodiscard]] Path* find_path(const PathPoint& point) const;
  Path& add_path(std::unique_ptr<Path>&& path);
  std::unique_ptr<Path> remove_path(const Path& path);
  [[nodiscard]] std::deque<PathPoint*> points() const;
  [[nodiscard]] std::deque<PathPoint*> selected_points() const;
  void deselect_all_points() const;
  [[nodiscard]] PathObject* path_object() const;
  [[nodiscard]] DisjointPathPointSetForest& joined_points() const;
  void update_joined_points_geometry() const;
  void join_points_by_position(const std::vector<Vec2f>& positions) const;

  /**
   * @brief is_valid returns true if this path vector is valid.
   * A PathVector must be valid before and after every public member function.
   * Undefined Behaviour may occur, if a path vector is not valid.
   * This function is a very convenient debugging tool, it shouldn't be used otherwise.
   */
  [[nodiscard, maybe_unused]] bool is_valid() const;

private:
  PathObject* m_path_object = nullptr;
  DisjointPathPointSetForest* m_shared_joined_points = nullptr;
  std::unique_ptr<DisjointPathPointSetForest> m_owned_joined_points;
  std::deque<std::unique_ptr<Path>> m_paths;
};

}  // namespace omm
