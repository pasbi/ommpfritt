#include "path/pathvector.h"

#include "commands/modifypointscommand.h"
#include "common.h"
#include "geometry/point.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "scene/disjointpathpointsetforest.h"
#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "path/path.h"
#include "path/graph.h"
#include "path/face.h"
#include "scene/mailbox.h"
#include "removeif.h"
#include <QObject>

namespace
{

constexpr auto JOINED_POINTES_SHARED = "joined_points_shared";
constexpr auto OWNED_JOINED_POINTS = "owned_joined_points";

using namespace omm;

std::map<PathPoint*, PathPoint*> map_points(const PathVector& from, const PathVector& to)
{
  const auto from_paths = from.paths();
  const auto to_paths = to.paths();
  assert(from_paths.size() == to_paths.size());
  std::map<PathPoint*, PathPoint*> map;
  for (std::size_t i = 0; i < from_paths.size(); ++i) {
    const auto& from_path = from_paths.at(i);
    const auto& to_path = to_paths.at(i);
    assert(from_path->size() == to_path->size());
    for (std::size_t j = 0; j < from_path->size(); ++j) {
      map.insert({&from_path->at(j), &to_path->at(j)});
    }
  }
  return map;
}

}  // namespace

namespace omm
{

class Style;

PathVector::PathVector(PathObject* path_object)
  : m_path_object(path_object)
  , m_owned_joined_points(std::make_unique<DisjointPathPointSetForest>())
{
}

bool PathVector::joined_points_shared() const
{
  assert((m_shared_joined_points == nullptr) != (m_owned_joined_points.get() == nullptr));
  return m_owned_joined_points == nullptr;
}

PathVector::PathVector(const PathVector& other, PathObject* path_object)
  : m_path_object(path_object)
  , m_owned_joined_points(std::make_unique<DisjointPathPointSetForest>(other.joined_points()))
{
  for (const auto* path : other.paths()) {
    add_path(std::make_unique<Path>(*path, this));
  }
  m_owned_joined_points->replace(map_points(other, *this));
}

PathVector::PathVector(PathVector&& other) noexcept
{
  swap(*this, other);
}

PathVector& PathVector::operator=(const PathVector& other)
{
  *this = PathVector{other};
  return *this;
}

std::unique_ptr<DisjointPathPointSetForest> PathVector::share_joined_points(DisjointPathPointSetForest& joined_points)
{
  assert(!joined_points_shared());
  m_shared_joined_points = &joined_points;
  for (const auto& set : m_owned_joined_points->sets()) {
    m_shared_joined_points->insert(set);
  }
  return std::move(m_owned_joined_points);
}

void PathVector::unshare_joined_points(std::unique_ptr<DisjointPathPointSetForest> joined_points)
{
  assert(joined_points_shared());
  m_shared_joined_points = nullptr;
  m_owned_joined_points = std::move(joined_points);
}

PathVector& PathVector::operator=(PathVector&& other) noexcept
{
  swap(*this, other);
  return *this;
}

void swap(PathVector& a, PathVector& b) noexcept
{
  swap(a.m_owned_joined_points, b.m_owned_joined_points);
  std::swap(a.m_path_object, b.m_path_object);
  swap(a.m_paths, b.m_paths);
  for (auto& path : a.m_paths) {
    path->set_path_vector(&a);
  }
  for (auto& path : b.m_paths) {
    path->set_path_vector(&b);
  }
  std::swap(a.m_shared_joined_points, b.m_shared_joined_points);
}

PathVector::~PathVector() = default;

void PathVector::serialize(serialization::SerializerWorker& worker) const
{
  worker.sub(SEGMENTS_POINTER)->set_value(m_paths, [](const auto& path, auto& worker_i) {
    if (path->size() == 0) {
      LWARNING << "Ignoring empty sub-path.";
    } else {
      path->serialize(worker_i);
    }
  });
  const bool shared = joined_points_shared();
  worker.sub(JOINED_POINTES_SHARED)->set_value(shared);
  if (!shared) {
    worker.sub(OWNED_JOINED_POINTS)->set_value(*m_owned_joined_points);
  }
}

void PathVector::deserialize(serialization::DeserializerWorker& worker)
{
  m_paths.clear();
  worker.sub(SEGMENTS_POINTER)->get_items([this](auto& worker_i) {
    Path& path = *m_paths.emplace_back(std::make_unique<Path>(this));
    path.deserialize(worker_i);
  });
  const bool shared = worker.sub(JOINED_POINTES_SHARED)->get_bool();
  if (!shared) {
    m_owned_joined_points = std::make_unique<DisjointPathPointSetForest>();
    worker.sub(OWNED_JOINED_POINTS)->get(*m_owned_joined_points);
  }
}

PathPoint& PathVector::point_at_index(std::size_t index) const
{
  for (Path* path : paths()) {
    if (index < path->size()) {
      return path->at(index);
    } else {
      index -= path->size();
    }
  }
  throw std::runtime_error{"Index out of bounds."};
}

QPainterPath PathVector::outline() const
{
  QPainterPath outline;
  for (const Path* path : paths()) {
    const auto points = path->points();
    if (!points.empty()) {
      outline.addPath(Path::to_painter_path(util::transform(points, [](const PathPoint* p) {
        return p->geometry();
      })));
    }
  }
  return outline;
}

std::vector<QPainterPath> PathVector::faces() const
{
  Graph graph{*this};
  graph.remove_articulation_edges();
  const auto faces = graph.compute_faces();
  std::vector<QPainterPath> qpps;
  qpps.reserve(faces.size());
  for (const auto& face : faces) {
    qpps.emplace_back(Path::to_painter_path(face.points()));
  }

  for (bool path_changed = true; path_changed;)
  {
    path_changed = false;
    for (auto& q1 : qpps) {
      for (auto& q2 : qpps) {
        if (&q1 == &q2) {
          continue;
        }
        if (q1.contains(q2)) {
          path_changed = true;
          q1 -= q2;
        }
      }
    }
  }

  return qpps;
}

std::size_t PathVector::point_count() const
{
  return std::accumulate(cbegin(m_paths), cend(m_paths), 0, [](std::size_t n, auto&& path) {
    return n + path->size();
  });
}

std::deque<Path*> PathVector::paths() const
{
  return util::transform(m_paths, std::mem_fn(&std::unique_ptr<Path>::get));
}

Path* PathVector::find_path(const PathPoint& point) const
{
  for (auto&& path : m_paths) {
    if (path->contains(point)) {
      return path.get();
    }
  }
  return nullptr;
}

Path& PathVector::add_path(std::unique_ptr<Path>&& path)
{
  path->set_path_vector(this);
  return *m_paths.emplace_back(std::move(path));
}

std::unique_ptr<Path> PathVector::remove_path(const Path &path)
{
  const auto it = std::find_if(m_paths.begin(), m_paths.end(), [&path](const auto& s_ptr) {
    return &path == s_ptr.get();
  });
  std::unique_ptr<Path> extracted_path;
  std::swap(extracted_path, *it);
  m_paths.erase(it);
  return extracted_path;
}

std::deque<PathPoint*> PathVector::points() const
{
  std::deque<PathPoint*> points;
  for (const auto& path : m_paths) {
    const auto& ps = path->points();
    points.insert(points.end(), ps.begin(), ps.end());
  }
  return points;
}

std::deque<PathPoint*> PathVector::selected_points() const
{
  return util::remove_if(points(), [](const auto& p) { return !p->is_selected(); });
}

void PathVector::deselect_all_points() const
{
  for (auto* point : points()) {
    point->set_selected(false);
  }
}

void PathVector::update_joined_points_geometry() const
{
  std::set<PathVector*> updated_path_vectors;
  for (auto* point : points()) {
    for (auto* buddy : point->joined_points()) {
      if (buddy != point && buddy->path_vector() != this) {
        updated_path_vectors.insert(buddy->path_vector());
        buddy->set_geometry(point->compute_joined_point_geometry(*buddy));
      }
    }
  }
  for (auto* path_vector : updated_path_vectors) {
    path_vector->path_object()->update();
  }
}

void PathVector::join_points_by_position(const std::vector<Vec2f>& positions) const
{
  static constexpr auto eps = 0.1;
  static constexpr auto eps2 = eps * eps;
  const auto points = this->points();
  LINFO << "===";
  for (const auto pos : positions) {
    ::transparent_set<PathPoint*> joint;
    for (auto* point : points) {
      const auto d2 = (point->geometry().position() - pos).euclidean_norm2();
      LINFO << "d2: " << d2 << "   " << point->geometry().position().to_string() << " " << pos.to_string();
      if (d2 < eps2) {
        joint.insert(point);
      }
    }
    joined_points().insert(joint);
  }
}

bool PathVector::is_valid() const
{
  if ((m_shared_joined_points == nullptr) != (!m_owned_joined_points)) {
    return false;
  }
  for (const auto& path : m_paths) {
    for (auto* point : path->points()) {
      if (&point->path() != path.get() || point->path_vector() != this) {
        return false;;
      }
    }
  }
  return true;
}

PathObject* PathVector::path_object() const
{
  return m_path_object;
}

DisjointPathPointSetForest& PathVector::joined_points() const
{
  if (joined_points_shared()) {
    return *m_shared_joined_points;
  } else {
    return *m_owned_joined_points;
  }
}

}  // namespace omm
