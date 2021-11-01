#pragma once

#include <deque>
#include <memory>
#include "aspects/serializable.h"

namespace Geom
{
class Path;
}  // namespace Geom

namespace omm
{

class Point;
class Segment;

struct SegmentView
{
public:
  explicit SegmentView(Segment& segment, std::size_t index, std::size_t size);
  Segment& segment;
  std::size_t index;
  std::size_t size;
};


class Segment : public Serializable
{

public:
  explicit Segment();
  explicit Segment(const Segment& other);
  explicit Segment(std::deque<std::unique_ptr<Point> >&& points);
  explicit Segment(const Geom::Path& path, bool is_closed);
  ~Segment();
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  std::size_t size() const;
  Point& at(std::size_t i) const;
  bool contains(const Point& point) const;
  std::size_t find(const Point& point) const;
  Point& add_point(const Vec2f& pos);
  Geom::Path to_geom_path(InterpolationMode interpolation, bool is_closed) const;
  void smoothen(bool is_closed);
  void smoothen_point(std::size_t i, bool is_closed);
  std::deque<Point*> points() const;
  void insert_points(std::size_t i, std::deque<std::unique_ptr<Point>> points);
  std::deque<std::unique_ptr<Point>> extract(std::size_t i, std::size_t n);

private:
  std::deque<std::unique_ptr<Point>> m_points;
};

}  // namespace
