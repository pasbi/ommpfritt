#include "path/pathvector.h"

#include "commands/modifypointscommand.h"
#include "common.h"
#include "geometry/point.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "objects/pathobject.h"
#include "path/edge.h"
#include "path/pathpoint.h"
#include "path/path.h"
#include "path/pathvectorgeometry.h"
#include "path/graph.h"
#include "path/face.h"
#include "scene/mailbox.h"
#include "removeif.h"
#include <QObject>
#include <QPainter>

namespace omm
{

class Style;

PathVector::PathVector(PathObject* path_object)
  : m_path_object(path_object)
{
}

PathVector::PathVector(const PathVectorGeometry& geometry, PathObject* path_object)
    : m_path_object(path_object)
    , m_paths(util::transform<std::deque>(geometry.paths(), [this](const auto& path_geometry) {
        return std::make_unique<Path>(path_geometry, this);
      }))
{

}

PathVector::PathVector(const PathVector& other, PathObject* path_object)
  : m_path_object(path_object)
{
  for (const auto* path : other.paths()) {
    add_path(std::make_unique<Path>(*path, this));
  }
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

PathVector& PathVector::operator=(PathVector&& other) noexcept
{
  swap(*this, other);
  return *this;
}

void swap(PathVector& a, PathVector& b) noexcept
{
  std::swap(a.m_path_object, b.m_path_object);
  swap(a.m_paths, b.m_paths);
  for (auto& path : a.m_paths) {
    path->set_path_vector(&a);
  }
  for (auto& path : b.m_paths) {
    path->set_path_vector(&b);
  }
}

PathVector::~PathVector() = default;

void PathVector::serialize(serialization::SerializerWorker& worker) const
{
  using PointIndices = std::map<const PathPoint*, std::size_t>;
  PointIndices point_indices;
  std::vector<std::vector<std::size_t>> iss;
  iss.reserve(m_paths.size());
  std::map<const Path*, std::size_t> path_indices;
  for (const auto& path : m_paths) {
    std::list<std::size_t> is;
    for (const auto* const point : path->points()) {
      const auto [it, was_inserted] = point_indices.try_emplace(point, point_indices.size());
      is.emplace_back(it->second);
    }
    iss.emplace_back(is.begin(), is.end());
    path_indices.emplace(path.get(), path_indices.size());
  }

  std::vector<const PathPoint*> point_indices_vec(point_indices.size());
  for (const auto& [point, index] : point_indices) {
    point_indices_vec.at(index) = point;
  }

  worker.sub("geometries")->set_value(point_indices_vec, [&path_indices](const PathPoint* const point, auto& worker) {
    point->geometry().serialize(worker, path_indices);
  });

  worker.sub("paths")->set_value(iss);
}

void PathVector::deserialize(serialization::DeserializerWorker& worker)
{
  std::vector<std::vector<std::size_t>> iss;
  worker.sub("paths")->get(iss);

  std::map<Path*, std::vector<std::size_t>> point_indices_per_path;
  std::vector<const Path*> paths;
  paths.reserve(iss.size());
  for (const auto& path_point_indices : iss) {
    auto& path = add_path();
    point_indices_per_path.emplace(&path, path_point_indices);
    paths.emplace_back(&path);
  }

  std::deque<std::shared_ptr<PathPoint>> points;
  worker.sub("geometries")->get_items([this, &paths, &points](auto& worker) {
    Point geometry;
    geometry.deserialize(worker, paths);
    points.emplace_back(std::make_shared<PathPoint>(geometry, this));
  });

  for (const auto& [path, point_indices] : point_indices_per_path) {
    if (point_indices.size() == 1) {
      path->set_single_point(points.at(point_indices.front()));
    } else {
      for (std::size_t i = 1; i < point_indices.size(); ++i) {
        const auto& a = points.at(point_indices.at(i - 1));
        const auto& b = points.at(point_indices.at(i));
        path->add_edge(std::make_unique<Edge>(a, b, path));
      }
    }
  }
}

std::set<Face> PathVector::faces() const
{
  Graph graph{*this};
  graph.remove_articulation_edges();
  return graph.compute_faces();
}

std::size_t PathVector::point_count() const
{
  return std::accumulate(cbegin(m_paths), cend(m_paths), 0, [](std::size_t n, const auto& path) {
    return n + path->points().size();
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

Path& PathVector::add_path(std::unique_ptr<Path> path)
{
  path->set_path_vector(this);
  return *m_paths.emplace_back(std::move(path));
}

Path& PathVector::add_path()
{
  return add_path(std::make_unique<Path>(this));
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

std::shared_ptr<PathPoint> PathVector::share(const PathPoint& path_point) const
{
  for (const auto& path : m_paths) {
    if (const auto& a = path->share(path_point); a != nullptr) {
      return a;
    }
  }
  return {};
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

void PathVector::draw_point_ids(QPainter& painter) const
{
  for (const auto* point : points()) {
    static constexpr QPointF offset{10.0, 10.0};
    painter.drawText(point->geometry().position().to_pointf() + offset, point->debug_id());
  }
}

PathVectorGeometry PathVector::geometry() const
{
  return PathVectorGeometry{util::transform<std::vector>(m_paths, [](const auto& path) {
    return path->geometry();
  })};
}

PathObject* PathVector::path_object() const
{
  return m_path_object;
}

}  // namespace omm
