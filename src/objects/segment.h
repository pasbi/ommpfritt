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


class Segment : public Serializable
{

public:
  explicit Segment();
  explicit Segment(const Segment& other);
  explicit Segment(std::deque<Point>&& points);
  explicit Segment(std::vector<Point>&& points);
  explicit Segment(const Geom::Path& path, bool is_closed);
  ~Segment();

  static constexpr auto POINTS_POINTER = "points";

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  std::size_t size() const;
  PathPoint& at(std::size_t i) const;
  bool contains(const PathPoint& point) const;
  std::size_t find(const PathPoint& point) const;
  PathPoint& add_point(const Vec2f& pos);
  Geom::Path to_geom_path(bool is_closed, InterpolationMode interpolation = InterpolationMode::Bezier) const;
  void smoothen(bool is_closed) const;
  Point smoothen_point(std::size_t i, bool is_closed) const;
  std::deque<PathPoint*> points() const;
  void insert_points(std::size_t i, std::deque<std::unique_ptr<PathPoint> >&& points);
  std::deque<std::unique_ptr<PathPoint>> extract(std::size_t i, std::size_t n);
  static std::vector<Geom::Point> compute_control_points(const Point& a, const Point& b,
                                                         InterpolationMode interpolation);

private:
  std::deque<std::unique_ptr<PathPoint>> m_points;
};

}  // namespace
