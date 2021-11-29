#include "path/pathvector.h"

#include "commands/modifypointscommand.h"
#include "common.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "path/path.h"
#include "path/enhancedpathvector.h"
#include <QObject>
#include "scene/mailbox.h"

namespace
{

using namespace omm;

auto copy(const std::deque<std::unique_ptr<Path>>& vs, PathVector* path_vector)
{
  std::decay_t<decltype(vs)> copy;
  for (auto&& v : vs) {
    std::cout << "v = " << v.get() << std::endl;
    copy.emplace_back(std::make_unique<Path>(*v, path_vector));
  }
  return copy;
}

using MaybeOwningDisjointPathPointSetForest = std::variant<std::unique_ptr<DisjointPathPointSetForest>,
                                                          DisjointPathPointSetForest*>;

MaybeOwningDisjointPathPointSetForest copy(const MaybeOwningDisjointPathPointSetForest& o)
{
  return std::visit([]<typename T>(const T& o) {
    if constexpr (std::is_same_v<T, DisjointPathPointSetForest*>) {
      return MaybeOwningDisjointPathPointSetForest(o);
    } else {
      return MaybeOwningDisjointPathPointSetForest(std::make_unique<DisjointPathPointSetForest>());
    }
  }, o);
}

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

PathVector::PathVector(PathObject& path_object, DisjointPathPointSetForest& shared_joined_points)
  : m_path_object(&path_object)
  , m_joined_points(&shared_joined_points)
{
}

PathVector::PathVector()
  : m_joined_points(std::make_unique<DisjointPathPointSetForest>())
{

}

PathVector::PathVector(const PathVector& other, PathObject* path_object)
  : m_path_object(path_object)
  , m_joined_points(copy(other.m_joined_points))
  , m_paths(copy(other.m_paths, this))
{
  const auto keep_old = std::holds_alternative<DisjointPathPointSetForest*>(m_joined_points);
  joined_points().replace(map_points(other, *this), keep_old);
}

PathVector::~PathVector() = default;

void PathVector::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  const auto paths_pointer = make_pointer(root, SEGMENTS_POINTER);
  serializer.start_array(m_paths.size(), paths_pointer);
  for (std::size_t i = 0; i < m_paths.size(); ++i) {
    if (m_paths.empty()) {
      LWARNING << "Ignoring empty sub-path.";
    } else {
      m_paths[i]->serialize(serializer, make_pointer(paths_pointer, i));
    }
  }
  serializer.end_array();
}

void PathVector::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  const auto paths_pointer = make_pointer(root, SEGMENTS_POINTER);
  const std::size_t n_paths = deserializer.array_size(paths_pointer);
  m_paths.clear();
  for (std::size_t i = 0; i < n_paths; ++i) {
    Path& path = *m_paths.emplace_back(std::make_unique<Path>(this));
    path.deserialize(deserializer, make_pointer(paths_pointer, i));
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

void PathVector::set(const Geom::PathVector& path_vector)
{
  if (m_path_object != nullptr) {
    m_path_object->update();
  }
  for (const auto& path : path_vector) {
    m_paths.emplace_back(std::make_unique<Path>(path, this));
  }
}

std::size_t PathVector::point_count() const
{
  return std::accumulate(cbegin(m_paths), cend(m_paths), 0, [](std::size_t n, auto&& path) {
    return n + path->size();
  });
}

std::deque<Path*> PathVector::paths() const
{
  return ::transform<Path*>(m_paths, std::mem_fn(&std::unique_ptr<Path>::get));
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
  return ::filter_if(points(), std::mem_fn(&PathPoint::is_selected));
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

PathObject* PathVector::path_object() const
{
  return m_path_object;
}

DisjointPathPointSetForest& PathVector::joined_points() const
{
  const auto visitor = [](const auto& joined_points) -> DisjointPathPointSetForest& {
    return *joined_points;
  };
  return std::visit(visitor, m_joined_points);
}

}  // namespace omm
