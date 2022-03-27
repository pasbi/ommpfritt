#pragma once

#include "common.h"
#include "geometry/vec2.h"
#include <deque>
#include <memory>
#include <QPainterPath>

namespace omm
{

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

class Point;
class PathPoint;

// NOLINTNEXTLINE(bugprone-forward-declaration-namespace)
class Path;

// NOLINTNEXTLINE(bugprone-forward-declaration-namespace)
class PathVector;

// NOLINTNEXTLINE(bugprone-forward-declaration-namespace)
class Path
{
public:
  explicit Path(PathVector* path_vector = nullptr);
  explicit Path(const Path& other, PathVector* path_vector = nullptr);
  explicit Path(std::deque<Point>&& points, PathVector* path_vector = nullptr);
  explicit Path(std::vector<Point>&& points, PathVector* path_vector = nullptr);
  ~Path();
  Path(Path&&) = delete;
  Path& operator=(const Path&) = delete;
  Path& operator=(Path&&) = delete;

  static constexpr auto POINTS_POINTER = "points";

  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);
  [[nodiscard]] std::size_t size() const;
  [[nodiscard]] PathPoint& at(std::size_t i) const;
  [[nodiscard]] bool contains(const PathPoint& point) const;
  [[nodiscard]] std::size_t find(const PathPoint& point) const;
  PathPoint& add_point(const Point& point);
  void make_linear() const;
  void smoothen() const;
  [[nodiscard]] Point smoothen_point(std::size_t i) const;
  [[nodiscard]] std::deque<PathPoint*> points() const;
  void insert_points(std::size_t i, std::deque<std::unique_ptr<PathPoint> >&& points);
  [[nodiscard]] std::deque<std::unique_ptr<PathPoint>> extract(std::size_t start, std::size_t size);
  [[nodiscard]] static std::vector<Vec2f>
  compute_control_points(const Point& a, const Point& b, InterpolationMode interpolation = InterpolationMode::Bezier);

  [[nodiscard]] PathVector* path_vector() const;
  void set_path_vector(PathVector* path_vector);
  void set_interpolation(InterpolationMode interpolation) const;

  template<typename Points> static QPainterPath to_painter_path(const Points& points, bool close = false)
  {
    if (points.empty()) {
      return {};
    }
    QPainterPath path;
    path.moveTo(points.front().position().to_pointf());
    for (auto it = points.begin(); next(it) != points.end(); ++it) {
      path.cubicTo(it->right_position().to_pointf(),
                   next(it)->left_position().to_pointf(),
                   next(it)->position().to_pointf());
    }
    if (close) {
      path.cubicTo(points.back().right_position().to_pointf(),
                   points.front().left_position().to_pointf(),
                   points.front().position().to_pointf());
    }
    return path;
  }

private:
  std::deque<std::unique_ptr<PathPoint>> m_points;
  PathVector* m_path_vector;
};

}  // namespace
