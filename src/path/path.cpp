#include "path/path.h"
#include "geometry/point.h"
#include "path/edge.h"
#include "path/pathpoint.h"
#include "path/pathview.h"
#include "path/pathview.h"
#include "path/pathgeometry.h"
#include <2geom/pathvector.h>

#include <QPainterPath>


namespace
{

/**
 * @brief replace_tangents_key replaces the key `{old_path, *}` with `{new_path, *}`
 *  or adds null-tangents at `{new_path, *}` (`*` stands for both directions).
 */
void replace_tangents_key(omm::Point& p, const omm::Path* old_path, const omm::Path& new_path)
{
  auto& tangents = p.tangents();
  for (const auto& direction : {omm::Point::Direction::Backward, omm::Point::Direction::Forward}) {
    const auto node = tangents.extract({old_path, direction});
    tangents.try_emplace({&new_path, direction}, node.empty() ? omm::PolarCoordinates() : node.mapped());
  }
}

void replace_tangents_key(const auto& edges, const omm::Path* old_path, const omm::Path& new_path)
{
  for (auto& edge : edges) {
    for (auto& p : {edge->a(), edge->b()}) {
      replace_tangents_key(p->geometry(), old_path, new_path);
    }
  }
}

}  // namespace

namespace omm
{

Path::Path(PathVector* path_vector)
    : m_path_vector(path_vector)
{
}

Path::Path(const Path& path, PathVector* path_vector)
    : m_edges(::copy(path.m_edges))
    , m_path_vector(path_vector)
{
  ::replace_tangents_key(m_edges, &path, *this);
  set_last_point_from_edges();
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
  replace_tangents_key(m_last_point->geometry(), nullptr, *this);
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
  auto lines = util::transform<QList>(this->edges(), &Edge::label);
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
  ::replace_tangents_key(std::vector{edge.get()}, nullptr, *this);

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

  ::replace_tangents_key(edges, nullptr, *this);

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

//std::tuple<std::unique_ptr<Edge>, Edge*, Edge*> Path::cut(Edge& edge, std::shared_ptr<PathPoint> p)
//{
//  const auto it = std::find_if(m_edges.begin(), m_edges.end(), [&edge](const auto& u) {
//    return u.get() == &edge;
//  });
//  if (it == m_edges.end()) {
//    throw PathException("Edge not found.");
//  }

//  const auto insert = [this](const auto pos, auto edge) -> Edge& {
//    auto& r = *edge;
//    m_edges.insert(pos, std::move(edge));
//    return r;
//  };
//  auto& r1 = insert(std::next(it, 1), std::make_unique<Edge>(edge.a(), p, this));
//  auto& r2 = insert(std::next(it, 2), std::make_unique<Edge>(p, edge.b(), this));

//  auto removed_edge = std::move(*it);
//  m_edges.erase(it);
//  assert(is_valid());
//  return {std::move(removed_edge), &r1, &r2};
//}

bool Path::is_valid() const
{
  if (m_last_point && !m_edges.empty() && m_edges.back()->b() != m_last_point) {
    LERROR << "Is not valid because last point is inconsistent.";
    return false;
  }

  const auto all_points_have_tangents = std::all_of(m_edges.begin(), m_edges.end(), [this](const auto& edge) {
    return edge->a()->geometry().tangents().contains({this, Point::Direction::Backward})
        && edge->a()->geometry().tangents().contains({this, Point::Direction::Forward})
        && edge->b()->geometry().tangents().contains({this, Point::Direction::Backward})
        && edge->b()->geometry().tangents().contains({this, Point::Direction::Forward});
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

PathGeometry Path::geometry() const
{
  auto ps = util::transform(points(), [this](const auto* const point) {
    auto g = point->geometry();
    auto tangents = g.tangents();
    std::erase_if(tangents, [this](const auto& pair) {
      return pair.first.path != this;
    });
    g.tangents().clear();
    for (const auto& [key, value] : tangents) {
      g.set_tangent({nullptr, key.direction}, value);
    };
    return g;
  });
  return PathGeometry(std::move(ps));
}

QPainterPath Path::to_painter_path() const
{
  const auto points = this->points();
  if (points.empty()) {
    return {};
  }
  QPainterPath path;
  path.moveTo(points.front()->geometry().position().to_pointf());
  for (std::size_t i = 1; i < points.size(); ++i) {
    const auto g1 = points.at(i - 1)->geometry();
    const auto g2 = points.at(i)->geometry();
    path.cubicTo(g1.tangent_position({this, Point::Direction::Forward}).to_pointf(),
                 g2.tangent_position({this, Point::Direction::Backward}).to_pointf(),
                 g2.position().to_pointf());
  }
  return path;
}


}  // namespace omm
