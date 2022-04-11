#include "path/path.h"
#include "geometry/point.h"
#include "path/edge.h"
#include "path/pathpoint.h"
#include "path/pathview.h"
#include "serializers/abstractserializer.h"
#include "serializers/serializerworker.h"
#include "serializers/deserializerworker.h"
#include "pathview.h"
#include <2geom/pathvector.h>

namespace
{

using namespace omm;




}  // namespace

namespace omm
{

Path::Path(PathVector* path_vector)
    : m_path_vector(path_vector)
{

}

Path::Path(const Path& path, PathVector* path_vector)
    : m_path_vector(path_vector)
{
  (void) path;  // TODO
}

Path::~Path() = default;

bool Path::contains(const PathPoint& point) const
{
  const auto points = this->points();
  return std::find(points.begin(), points.end(), &point) != points.end();
}

void Path::make_linear() const
{
  for (const auto& point : points()) {
    point->set_geometry(point->geometry().nibbed());
  }
}

void Path::set_interpolation(InterpolationMode interpolation) const
{
  switch (interpolation) {
  case InterpolationMode::Bezier:
    return;
  case InterpolationMode::Smooth:
    smoothen();
    return;
  case InterpolationMode::Linear:
    make_linear();
    return;
  }
  Q_UNREACHABLE();
}

QPainterPath Path::to_painter_path() const
{
  if (const auto points = this->points(); !points.empty()) {
    return Path::to_painter_path(util::transform(points, &PathPoint::geometry));
  } else {
    return {};
  }
}

std::vector<Vec2f> Path::compute_control_points(const Point& a, const Point& b, InterpolationMode interpolation)
{
  static constexpr double t = 1.0 / 3.0;
  switch (interpolation) {
  case InterpolationMode::Bezier:
    [[fallthrough]];
  case InterpolationMode::Smooth:
    return {a.position(),
            a.right_position(),
            b.left_position(),
            b.position()};
    break;
  case InterpolationMode::Linear:
    return {a.position(),
            ((1.0 - t) * a.position() + t * b.position()),
            ((1.0 - t) * b.position() + t * a.position()),
            b.position()};
    break;
  }
  Q_UNREACHABLE();
  return {};
}

PathVector* Path::path_vector() const
{
  return m_path_vector;
}

void Path::set_path_vector(PathVector* path_vector)
{
  m_path_vector = path_vector;
}

void Path::smoothen() const
{
}

Point Path::smoothen_point(std::size_t i) const
{
  Q_UNUSED(i)
  return {};
}

void Path::serialize(serialization::SerializerWorker& worker) const
{
  (void) worker;
}

void Path::deserialize(serialization::DeserializerWorker& worker)
{
  (void) worker;
}

Edge& Path::add_edge(std::unique_ptr<Edge> edge)
{
  const auto try_emplace = [this](std::unique_ptr<Edge>& edge) {
    if (m_edges.empty() || m_edges.back()->b() == edge->a()) {
      m_edges.emplace_back(std::move(edge));
    } else if (m_edges.front()->a() == edge->b()) {
      m_edges.emplace_front(std::move(edge));
    } else {
      return false;
    }
    return true;
  };

  auto& ref = *edge;

  if (!try_emplace(edge)) {
    edge->flip();
    if (!try_emplace(edge)) {
      throw PathException{"Cannot add edge to path."};
    }
  }

  return ref;
}

std::pair<std::deque<std::unique_ptr<Edge>>, Edge*> Path::remove(const PathView& path_view, std::unique_ptr<Edge> bridge)
{
  const auto first = std::next(m_edges.begin(), std::max(static_cast<std::size_t>(1), path_view.begin()) - 1);
  const auto last = std::next(m_edges.begin(), std::min(path_view.end(), m_edges.size()));

  std::deque<std::unique_ptr<Edge>> removed_edges;
  Edge* new_edge = nullptr;

  std::copy(std::move_iterator{first}, std::move_iterator{last}, std::back_inserter(removed_edges));

  if (path_view.begin() > 0 && path_view.begin() + path_view.size() < m_edges.size()) {
    const auto& previous_edge = *std::next(first, -1);
    const auto& next_edge = *std::next(last, 1);
    auto new_edge_own = [&previous_edge, &next_edge, &bridge, this]() {
      if (bridge == nullptr) {
        return std::make_unique<Edge>(previous_edge->b(), next_edge->a(), this);
      } else {
        assert(bridge->a() == previous_edge->b());
        assert(bridge->b() == next_edge->a());
        assert(bridge->path() == this);
        return std::move(bridge);
      }
    }();
    new_edge = new_edge_own.get();
    m_edges.insert(std::next(last), std::move(new_edge_own));
  } else {
    assert(bridge == nullptr);
  }
  m_edges.erase(first, last);
  assert(is_valid());
  return {std::move(removed_edges), new_edge};
}

std::deque<std::unique_ptr<Edge> > Path::replace(const PathView& path_view, std::deque<std::unique_ptr<Edge>> edges)
{
  assert(is_valid());
  std::deque<std::unique_ptr<Edge>> removed;
  const auto begin = std::next(m_edges.begin(), path_view.begin());
  const auto end = std::next(begin, path_view.size());
  std::copy(std::move_iterator{begin}, std::move_iterator{end}, std::back_inserter(removed));
  m_edges.erase(begin, end);
  m_edges.insert(std::next(begin, path_view.begin()),
                 std::move_iterator{edges.begin()},
                 std::move_iterator{edges.end()});
  assert(is_valid());
  return removed;
}

std::tuple<std::unique_ptr<Edge>, Edge*, Edge*> Path::cut(Edge& edge, std::shared_ptr<PathPoint> p)
{
  const auto it = std::find_if(m_edges.begin(), m_edges.end(), [&edge](const auto& u) {
    return u.get() == &edge;
  });
  if (it == m_edges.end()) {
    throw PathException("Edge not found.");
  }

  const auto insert = [this](const auto pos, auto edge) -> Edge& {
    auto& r = *edge;
    m_edges.insert(pos, std::move(edge));
    return r;
  };
  auto& r1 = insert(std::next(it, 1), std::make_unique<Edge>(edge.a(), p, this));
  auto& r2 = insert(std::next(it, 2), std::make_unique<Edge>(p, edge.b(), this));

  auto removed_edge = std::move(*it);
  m_edges.erase(it);
  assert(is_valid());
  return {std::move(removed_edge), &r1, &r2};
}

bool Path::is_valid() const
{
  return is_valid(m_edges);
}

std::vector<PathPoint*> Path::points() const
{
  if (m_edges.empty()) {
    return {};
  }

  std::vector<PathPoint*> points;
  points.reserve(m_edges.size() + 1);
  points.emplace_back(m_edges.front()->a().get());
  for (const auto& edge : m_edges) {
    points.emplace_back(edge->b().get());
  }
  return points;
}

std::vector<Edge*> Path::edges() const
{
  return util::transform<std::vector>(m_edges, &std::unique_ptr<Edge>::get);
}

}  // namespace omm
