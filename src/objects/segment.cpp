#include "geometry/point.h"
#include "objects/pathpoint.h"
#include "objects/segment.h"
#include "serializers/abstractserializer.h"
#include <2geom/pathvector.h>

namespace
{

using namespace omm;

auto copy(const std::deque<std::unique_ptr<PathPoint>>& vs, Segment& segment)
{
  std::decay_t<decltype(vs)> copy;
  for (auto&& v : vs) {
    copy.emplace_back(std::make_unique<PathPoint>(v->geometry(), segment));
  }
  return copy;
}

auto to_path_points(std::vector<Point>&& points, Segment& segment)
{
  return ::transform<std::unique_ptr<PathPoint>, std::deque>(std::move(points), [&segment](auto&& point) {
    return std::make_unique<PathPoint>(point, segment);
  });
}

auto to_path_points(std::deque<Point>&& points, Segment& segment)
{
  return ::transform<std::unique_ptr<PathPoint>>(std::move(points), [&segment](auto&& point) {
    return std::make_unique<PathPoint>(point, segment);
  });
}

}  // namespace

namespace omm
{

Segment::Segment(Path*)
{
}

Segment::Segment(std::deque<Point>&& points, Path* path)
  : m_points(to_path_points(std::move(points), *this))
  , m_path(path)
{
}

Segment::Segment(std::vector<Point>&& points, Path* path)
  : m_points(to_path_points(std::move(points), *this))
  , m_path(path)
{
}

Segment::Segment(const Geom::Path& geom_path, bool is_closed, Path* path)
  : m_path(path)
{
  const auto n = geom_path.size();
  for (std::size_t i = 0; i < n; ++i) {
    const auto& c = dynamic_cast<const Geom::CubicBezier&>(geom_path[i]);
    const auto p0 = Vec2f(c[0]);
    if (m_points.empty()) {
      m_points.push_back(std::make_unique<PathPoint>(Point{p0}, *this));
    }
    m_points.back()->geometry().set_right_tangent(PolarCoordinates(Vec2f(c[1]) - p0));
    const auto p1 = Vec2f(c[3]);
    auto& pref = *[wrap = is_closed && i == n - 1, p1, this]() -> decltype(auto) {
      if (wrap) {
        return m_points.front();
      } else {
        return m_points.emplace_back(std::make_unique<PathPoint>(Point{p1}, *this));
      }
    }();
    pref.geometry().set_left_tangent(PolarCoordinates(Vec2f(c[2]) - p1));
  }
  if (is_closed) {
    assert(geom_path.size() == m_points.size());
  } else {
    // path counts number of curves, segments counts number of points
    assert(geom_path.size() + 1 == m_points.size());
  }
}

Segment::Segment(const Segment& other, Path* path)
  : m_points(copy(other.m_points, *this))
  , m_path(path)
{
}

Segment::~Segment() = default;

std::size_t Segment::size() const
{
  return m_points.size();
}

PathPoint& Segment::at(std::size_t i) const
{
  return *m_points.at(i);
}

bool Segment::contains(const PathPoint& point) const
{
  return  std::any_of(m_points.begin(), m_points.end(), [&point](const auto& candidate) {
    return &point == candidate.get();
  });
}

std::size_t Segment::find(const PathPoint& point) const
{
  const auto it = std::find_if(m_points.begin(), m_points.end(), [&point](const auto& candidate) {
    return &point == candidate.get();
  });
  if (it == m_points.end()) {
    throw std::out_of_range("No such point in segment.");
  } else {
    return std::distance(m_points.begin(), it);
  }
}

PathPoint& Segment::add_point(const Vec2f& pos)
{
  return *m_points.emplace_back(std::make_unique<PathPoint>(Point{pos}, *this));
}

Geom::Path Segment::to_geom_path(bool is_closed, InterpolationMode interpolation) const
{
  std::vector<Geom::CubicBezier> bzs;
  const std::size_t n = m_points.size();
  if (n == 0) {
    return Geom::Path{};
  }
  const std::size_t m = is_closed ? n : n - 1;
  bzs.reserve(m);

  std::unique_ptr<Segment> smoothened;
  const Segment* self = this;
  if (interpolation == InterpolationMode::Smooth) {
    smoothened = std::make_unique<Segment>(*this);
    smoothened->smoothen(is_closed);
    self = smoothened.get();
  }

  for (std::size_t i = 0; i < m; ++i) {
    const std::size_t j = (i + 1) % n;
    bzs.push_back(compute_control_points(self->at(i).geometry(), self->at(j).geometry(), interpolation));
  }

  return {bzs.begin(), bzs.end(), is_closed};
}

std::vector<Geom::Point>
Segment::compute_control_points(const Point& a, const Point& b, InterpolationMode interpolation)
{
  static constexpr double t = 1.0 / 3.0;
  switch (interpolation) {
  case InterpolationMode::Bezier:
    [[fallthrough]];
  case InterpolationMode::Smooth:
    return {a.position().to_geom_point(),
            a.right_position().to_geom_point(),
            b.left_position().to_geom_point(),
            b.position().to_geom_point()};
    break;
  case InterpolationMode::Linear:
    return {a.position().to_geom_point(),
            ((1.0 - t) * a.position() + t * b.position()).to_geom_point(),
            ((1.0 - t) * b.position() + t * a.position()).to_geom_point(),
            b.position().to_geom_point()};
    break;
  }
  Q_UNREACHABLE();
  return {};
}

void Segment::smoothen(bool is_closed) const
{
  std::deque<std::unique_ptr<PathPoint>> points;
  for (std::size_t i = 0; i < m_points.size(); ++i) {
    Point smoothened_point = smoothen_point(i, is_closed);
    m_points[i]->geometry() =  smoothened_point;
  }
}

Point Segment::smoothen_point(std::size_t i, bool is_closed) const
{
  const std::size_t n = m_points.size();
  PathPoint* left;
  PathPoint* right;
  if (i == 0) {
    left = is_closed ? m_points.at(n - 1).get() : m_points.at(0).get();
    right = m_points.at(1).get();
  } else if (i == n - 1) {
    left = m_points.at(n - 2).get();
    right = is_closed ? m_points.at(0).get() : m_points.at(n - 1).get();
  } else {
    left = m_points.at(i - 1).get();
    right = m_points.at(i + 1).get();
  }
  Point copy = m_points[i]->geometry();
  const Vec2f d = (left->geometry().position() - right->geometry().position()) / 6.0;
  copy.set_right_tangent(PolarCoordinates(-d));
  copy.set_left_tangent(PolarCoordinates(d));
  return copy;
}

std::deque<PathPoint*> Segment::points() const
{
  return ::transform<PathPoint*>(m_points, [](const auto& pt) { return pt.get(); });
}

void Segment::insert_points(std::size_t i, std::deque<std::unique_ptr<PathPoint>>&& points)
{
  m_points.insert(m_points.begin() + i,
                  std::make_move_iterator(points.begin()),
                  std::make_move_iterator(points.end()));
}

std::deque<std::unique_ptr<PathPoint> > Segment::extract(std::size_t start, std::size_t size)
{
  std::deque<std::unique_ptr<PathPoint>> points(size);
  for (std::size_t i = 0; i < size; ++i) {
    std::swap(points[i], m_points[i + start]);
  }
  const auto begin = m_points.begin() + start;
  const auto end = begin + size;
  m_points.erase(begin, end);
  return points;
}

void Segment::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  const auto points_ptr = make_pointer(root, POINTS_POINTER);
  serializer.start_array(m_points.size(), points_ptr);
  for (std::size_t i = 0; i < m_points.size(); ++i) {
    m_points.at(i)->geometry().serialize(serializer, make_pointer(points_ptr, i));
  }
  serializer.end_array();
}

void Segment::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  const auto points_ptr = make_pointer(root, POINTS_POINTER);
  const auto size = deserializer.array_size(points_ptr);
  for (std::size_t i = 0; i < size; ++i) {
    PathPoint& point = *m_points.emplace_back(std::make_unique<PathPoint>(*this));
    point.geometry().deserialize(deserializer, make_pointer(points_ptr, i));
  }
}

SegmentView::SegmentView(Segment& segment, std::size_t index, std::size_t size)
  : segment(&segment), index(index), size(size)
{
}

bool operator<(const SegmentView& a, const SegmentView& b)
{
  static constexpr auto as_tuple = [](const SegmentView& a) {
    return std::tuple{a.segment, a.index};
  };
  return as_tuple(a) < as_tuple(b);
}

std::ostream& operator<<(std::ostream& ostream, const SegmentView& segment_view)
{
  ostream << "Segment[" << segment_view.segment << " " << segment_view.index << " " << segment_view.size << "]";
  return ostream;
}

}  // namespace omm
