#include "path/path.h"
#include "config.h"
#include "geometry/point.h"
#include "path/dedge.h"
#include "path/edge.h"
#include "path/pathpoint.h"
#include "path/pathview.h"

#include <QPainterPath>


namespace
{

void replace_tangents_key(const auto& edges, const std::map<const omm::Path*, omm::Path*>& map)
{
  for (auto& edge : edges) {
    for (auto& p : {edge->a(), edge->b()}) {
      p->geometry().replace_tangents_key(map);
    }
  }
}

[[maybe_unused]] void draw_arrow(QPainterPath& painter_path, const omm::PolarCoordinates& v, const double offset)
{
  const auto pos_before = painter_path.currentPosition();
  const auto origin = omm::Vec2f(pos_before) + offset * v.to_cartesian();
  const auto move_to = [v, &painter_path, origin](const double angle_offset) {
    const auto end_pos = (v + angle_offset).to_cartesian() + origin;
    painter_path.moveTo(origin.to_pointf());
    painter_path.lineTo(end_pos.to_pointf());
  };
  move_to(M_PI / 6.0);
  move_to(-M_PI / 6.0);
  painter_path.moveTo(pos_before);
}

}  // namespace

namespace omm
{

Path::Path(PathVector* path_vector)
    : m_path_vector(path_vector)
{
}

Path::~Path() = default;

bool Path::contains(const PathPoint& point) const
{
  const auto points = this->points();
  return std::find(points.begin(), points.end(), &point) != points.end();
}

std::shared_ptr<PathPoint> Path::share(const PathPoint& point) const
{
  if (m_edges.empty()) {
    if (m_last_point.get() == &point) {
      return m_last_point;
    } else {
      return {};
    }
  }

  if (const auto& a = m_edges.front()->a(); a.get() == &point) {
    return a;
  }

  for (const auto& edge : m_edges) {
    if (const auto& b = edge->b(); b.get() == &point) {
      return b;
    }
  }

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

void Path::set_single_point(std::shared_ptr<PathPoint> single_point)
{
  assert(m_edges.empty());
  assert(!m_last_point);
  m_last_point = single_point;
  m_last_point->geometry().replace_tangents_key({{nullptr, this}});
}

std::shared_ptr<PathPoint> Path::extract_single_point()
{
  assert(m_edges.empty());
  assert(m_last_point);
  auto last_point = m_last_point;
  m_last_point.reset();
  return last_point;
}

QString Path::print_edge_info() const
{
  auto lines = util::transform<QList>(this->edges(), &Edge::to_string);
  lines.push_front(QString::asprintf("== Path 0x%p, Edges %zu", static_cast<const void*>(this), edges().size()));
  if (edges().empty()) {
    lines.append(QString::asprintf("Single point: 0x%p", static_cast<const void*>(m_last_point.get())));
  }
  return lines.join("\n");
}

void Path::set_last_point_from_edges()
{
  if (m_edges.empty()) {
    m_last_point = nullptr;
  } else {
    m_last_point = m_edges.back()->b();
  }
}

Edge& Path::add_edge(std::shared_ptr<PathPoint> a, std::shared_ptr<PathPoint> b)
{
  return add_edge(std::make_unique<Edge>(a, b, this));
}

std::shared_ptr<PathPoint> Path::last_point() const
{
  return m_last_point;
}

std::shared_ptr<PathPoint> Path::first_point() const
{
  if (m_edges.empty()) {
    return m_last_point;
  } else {
    return m_edges.front()->a();
  }
}

Edge& Path::add_edge(std::unique_ptr<Edge> edge)
{
  assert(edge->a() && edge->b());
  ::replace_tangents_key(std::vector{edge.get()}, {{nullptr, this}});

  const auto try_emplace = [this](std::unique_ptr<Edge>& edge) {
    if (m_last_point == nullptr || last_point().get() == edge->a().get()) {
      m_edges.emplace_back(std::move(edge));
    } else if (first_point().get() == edge->b().get()) {
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

  set_last_point_from_edges();
  return ref;
}


std::deque<std::unique_ptr<Edge>> Path::replace(const PathView& path_view, std::deque<std::unique_ptr<Edge>> edges)
{
  assert(is_valid(edges));
  assert(is_valid());

  ::replace_tangents_key(edges, {{nullptr, this}});

  const auto swap_edges = [this](const auto& begin, const auto& end, std::deque<std::unique_ptr<Edge>>&& edges) {
    std::deque<std::unique_ptr<Edge>> removed_edges;
    std::copy(std::move_iterator(begin), std::move_iterator(end), std::back_inserter(removed_edges));
    auto gap_begin = m_edges.erase(begin, end);
    m_edges.insert(gap_begin, std::move_iterator(edges.begin()), std::move_iterator(edges.end()));
    return removed_edges;
  };

  const bool set_last_point_from_edges = [this, &edges, path_view]() {
    if (edges.empty() && path_view.point_count() == points().size() - 1) {
      // There will be no edges left but m_last_point needs to be set.

      if (path_view.begin() > 0) {
        // all edges will be removed and all points except the first one.
        m_last_point = first_point();
      } // else all edges will be removed and all points except the last one.

      return false;  // Don't update m_last_point later,
    } else {
      return true;  // Do update m_last_point later
    }
  }();

  std::deque<std::unique_ptr<Edge>> removed_edges;
  if (path_view.begin() == 0 && path_view.end() == points().size()) {
    // all points are replaced
    removed_edges = swap_edges(m_edges.begin(), m_edges.end(), std::move(edges));
  } else if (path_view.begin() == 0) {
    // append left
    removed_edges = swap_edges(m_edges.begin(), m_edges.begin() + path_view.point_count(), std::move(edges));
  } else if (path_view.end() == points().size()) {
    // append right
    removed_edges = swap_edges(m_edges.end() - path_view.point_count(), m_edges.end(), std::move(edges));
  } else {
    // append middle
    const auto begin = m_edges.begin() + path_view.begin() - 1;
    removed_edges = swap_edges(begin,
                               begin + path_view.point_count() + 1,
                               std::move(edges));
  }

  if (set_last_point_from_edges) {
    this->set_last_point_from_edges();
  }
  assert(is_valid());
  assert(is_valid(removed_edges));
  return removed_edges;
}

bool Path::is_valid() const
{
  if (m_last_point == nullptr) {
    if (m_edges.empty()) {
      return true;
    } else {
      LERROR << "Last point may only be null if path is empty, but path has edges.";
      return false;
    }
  }

  if (!m_edges.empty() && m_edges.back()->b() != m_last_point) {
    LERROR << "Is not valid because last point is inconsistent.";
    return false;
  }

  const auto all_points_have_tangents
      = std::all_of(m_edges.begin(), m_edges.end(), [this](const auto& edge) {
          return edge->a()->geometry().tangents().contains({this, Direction::Backward})
                 && edge->a()->geometry().tangents().contains({this, Direction::Forward})
                 && edge->b()->geometry().tangents().contains({this, Direction::Backward})
                 && edge->b()->geometry().tangents().contains({this, Direction::Forward});
        });
  return is_valid(m_edges) && all_points_have_tangents;
}

std::vector<PathPoint*> Path::points() const
{
  if (m_edges.empty()) {
    if (m_last_point) {
      return {m_last_point.get()};
    }
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

Edge& Path::edge(std::size_t i) const
{
  return *m_edges.at(i);
}

void Path::draw_segment(QPainterPath& painter_path, const Edge& edge, const Path* const path)
{

  const auto g1 = edge.a()->geometry();
  const auto t1 = g1.tangent_position({path, Direction::Forward});
  const auto g2 = edge.b()->geometry();
  const auto t2 = g2.tangent_position({path, Direction::Backward});

  const DEdgeConst dedge(&edge, Direction::Forward);

  static constexpr auto len = [](const auto& p1, const auto& p2) { return (p1 - p2).euclidean_norm(); };
  [[maybe_unused]] const auto arrow_size = (len(g1.position(), t1) + len(t1, t2) + len(t2, g2.position())) / 5.0;
  if constexpr (PATH_DRAW_DIRECTION) {
    const PolarCoordinates v(dedge.start_angle() + M_PI, arrow_size);
    draw_arrow(painter_path, 0.5 * v, -0.1);
  }

  painter_path.cubicTo(t1.to_pointf(), t2.to_pointf(), g2.position().to_pointf());

  if constexpr (PATH_DRAW_DIRECTION) {
    const PolarCoordinates v(dedge.end_angle(), arrow_size);
    draw_arrow(painter_path, v, 0.1);
  }
}

QPainterPath Path::to_painter_path() const
{
  if (m_edges.empty()) {
    return {};
  }
  QPainterPath painter_path;
  painter_path.moveTo(m_edges.front()->a()->geometry().position().to_pointf());
  for (const auto& edge : m_edges) {
    draw_segment(painter_path, *edge, this);
  }
  return painter_path;
}


}  // namespace omm
