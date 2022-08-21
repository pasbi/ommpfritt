#pragma once

#include <deque>
#include <map>
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
  PathVector& operator=(PathVector other) = delete;

  // Moving the path vector is not trivially possible because the owned PathPoint hold a pointer
  // to the owned Paths and this.
  // If you need to move a PathVector, encapsulated it in a unique_ptr.
  PathVector(PathVector&& other) = delete;
  PathVector& operator=(PathVector&& other) = delete;

  ~PathVector();

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
  [[nodiscard]] std::set<PathPoint*> points() const;
  [[nodiscard]] std::set<PathPoint*> selected_points() const;
  void deselect_all_points() const;
  [[nodiscard]] PathObject* path_object() const;
  void draw_point_ids(QPainter& painter) const;

  static std::unique_ptr<PathVector> join(const std::deque<PathVector*>& pvs, double eps);

  /**
   * @brief join joins the points @code ps
   * All points of @code ps must be part of this @code PathVector.
   * One point of @code ps is kept, a pointer to this one is returned.
   * All other points of @code are merged into that one and their ownership is returned.
   * It is unspecified which point is kept.
   * If @code ps is empty, nothing happens.
   */
  PathPoint* join(std::set<PathPoint*> ps);


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

  struct Mapping
  {
    std::map<const PathPoint*, PathPoint*> points;
    std::map<const Path*, Path*> paths;
  };

  /**
   * @brief adopt adopts all paths and points of @code pv by copying.
   *  There will be no reference to @code pv when this function is done.
   * @return a mapping from @code PathPoint in pv to @code PathPoint in `this`.
   */
  Mapping adopt(const PathVector& pv);
};

}  // namespace omm
