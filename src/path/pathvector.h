#pragma once

#include "aspects/serializable.h"
#include <variant>

class QPainterPath;

namespace omm
{

class EnhancedPathVector;
class Path;
class PathPoint;
class PathObject;
class JoinedPointIndices;
class DisjointPathPointSetForest;
class Scene;

class PathVector : public Serializable
{
public:
  explicit PathVector(PathObject& path_object, DisjointPathPointSetForest& shared_joined_points);
  explicit PathVector();
  explicit PathVector(const PathVector& other, PathObject* path_object);
  PathVector(const PathVector&) = delete;
  PathVector(PathVector&&) = delete;
  PathVector& operator=(const PathVector& other) = delete;
  PathVector& operator=(PathVector&&) = delete;
  ~PathVector() override;
  static constexpr auto SEGMENTS_POINTER = "segments";
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  [[nodiscard]] PathPoint& point_at_index(std::size_t index) const;
  [[nodiscard]] Geom::PathVector to_geom() const;
  void set(const Geom::PathVector& path_vector);

  [[nodiscard]] const QPainterPath& outline() const;
  [[nodiscard]] const QPainterPath& fill() const;
  [[nodiscard]] JoinedPointIndices joined_point_indices() const;
  [[nodiscard]] std::size_t point_count() const;
  [[nodiscard]] std::deque<Path*> paths() const;
  [[nodiscard]] Path* find_path(const PathPoint& point) const;
  Path& add_path(std::unique_ptr<Path>&& segment);
  std::unique_ptr<Path> remove_path(const Path& segment);
  [[nodiscard]] std::deque<PathPoint*> points() const;
  [[nodiscard]] std::deque<PathPoint*> selected_points() const;
  void deselect_all_points() const;
  [[nodiscard]] PathObject* path_object() const;
  [[nodiscard]] DisjointPathPointSetForest& joined_points() const;
  void update_joined_points_geometry() const;

private:
  PathObject* const m_path_object = nullptr;
  std::variant<std::unique_ptr<DisjointPathPointSetForest>, DisjointPathPointSetForest*> m_joined_points;
  std::deque<std::unique_ptr<Path>> m_paths;
};

}  // namespace omm
