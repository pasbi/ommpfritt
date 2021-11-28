#pragma once

#include <deque>
#include <memory>
#include "aspects/serializable.h"

namespace Geom
{
class Path;
class Curve;
class Point;
}  // namespace Geom

namespace omm
{

class Point;
class PathPoint;
class Path;

struct PathView
{
public:
  explicit PathView(Path& path, std::size_t index, std::size_t size);
  friend bool operator<(const PathView& a, const PathView& b);
  friend std::ostream& operator<<(std::ostream& ostream, const PathView& path_view);
  Path* path;
  std::size_t index;
  std::size_t size;
};

class PathVector;

class Path : public Serializable
{
public:
  explicit Path(PathVector* path_vector = nullptr);
  explicit Path(const Path& other, PathVector* path_vector = nullptr);
  explicit Path(std::deque<Point>&& points, PathVector* path_vector = nullptr);
  explicit Path(std::vector<Point>&& points, PathVector* path_vector = nullptr);
  explicit Path(const Geom::Path& geom_path, PathVector* path_vector = nullptr);
  ~Path() override;
  Path(Path&&) = delete;
  Path& operator=(const Path&) = delete;
  Path& operator=(Path&&) = delete;

  static constexpr auto POINTS_POINTER = "points";

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  [[nodiscard]] std::size_t size() const;
  [[nodiscard]] PathPoint& at(std::size_t i) const;
  [[nodiscard]] bool contains(const PathPoint& point) const;
  [[nodiscard]] std::size_t find(const PathPoint& point) const;
  [[nodiscard]] PathPoint& add_point(const Vec2f& pos);
  [[nodiscard]] Geom::Path to_geom_path(InterpolationMode interpolation = InterpolationMode::Bezier) const;
  void smoothen() const;
  [[nodiscard]] Point smoothen_point(std::size_t i) const;
  [[nodiscard]] std::deque<PathPoint*> points() const;
  void insert_points(std::size_t i, std::deque<std::unique_ptr<PathPoint> >&& points);
  [[nodiscard]] std::deque<std::unique_ptr<PathPoint>> extract(std::size_t start, std::size_t size);
  [[nodiscard]] static std::vector<Geom::Point>
  compute_control_points(const Point& a, const Point& b, InterpolationMode interpolation = InterpolationMode::Bezier);

  [[nodiscard]] PathVector* path_vector() const;

private:
  std::deque<std::unique_ptr<PathPoint>> m_points;
  PathVector* const m_path_vector;
};

}  // namespace
