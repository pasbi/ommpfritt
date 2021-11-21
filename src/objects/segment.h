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
class Segment;

struct SegmentView
{
public:
  explicit SegmentView(Segment& segment, std::size_t index, std::size_t size);
  friend bool operator<(const SegmentView& a, const SegmentView& b);
  friend std::ostream& operator<<(std::ostream& ostream, const SegmentView& segment_view);
  Segment* segment;
  std::size_t index;
  std::size_t size;
};

class Path;

class Segment : public Serializable
{
public:
  explicit Segment(Path* path = nullptr);
  explicit Segment(const Segment& other, Path* path = nullptr);
  explicit Segment(std::deque<Point>&& points, Path* path = nullptr);
  explicit Segment(std::vector<Point>&& points, Path* path = nullptr);
  explicit Segment(const Geom::Path& geom_path, bool is_closed, Path* path = nullptr);
  ~Segment() override;
  Segment(Segment&&) = delete;
  Segment& operator=(const Segment&) = delete;
  Segment& operator=(Segment&&) = delete;

  static constexpr auto POINTS_POINTER = "points";

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  [[nodiscard]] std::size_t size() const;
  [[nodiscard]] PathPoint& at(std::size_t i) const;
  [[nodiscard]] bool contains(const PathPoint& point) const;
  [[nodiscard]] std::size_t find(const PathPoint& point) const;
  [[nodiscard]] PathPoint& add_point(const Vec2f& pos);
  [[nodiscard]] Geom::Path to_geom_path(bool is_closed,
                                        InterpolationMode interpolation = InterpolationMode::Bezier) const;
  void smoothen(bool is_closed) const;
  [[nodiscard]] Point smoothen_point(std::size_t i, bool is_closed) const;
  [[nodiscard]] std::deque<PathPoint*> points() const;
  void insert_points(std::size_t i, std::deque<std::unique_ptr<PathPoint> >&& points);
  [[nodiscard]] std::deque<std::unique_ptr<PathPoint>> extract(std::size_t start, std::size_t size);
  [[nodiscard]] static std::vector<Geom::Point> compute_control_points(const Point& a,
                                                                       const Point& b,
                                                                       InterpolationMode interpolation);
  [[nodiscard]] Path* path() const;

private:
  std::deque<std::unique_ptr<PathPoint>> m_points;
  Path* const m_path;
};

}  // namespace
