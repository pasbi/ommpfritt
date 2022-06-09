#pragma once

#include "common.h"
#include "logging.h"
#include <deque>
#include <memory>

namespace omm
{

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

class Point;
class PathPoint;
class Edge;
class PathView;
class PathGeometry;

// NOLINTNEXTLINE(bugprone-forward-declaration-namespace)
class Path;

// NOLINTNEXTLINE(bugprone-forward-declaration-namespace)
class PathVector;

class PathException : std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

// NOLINTNEXTLINE(bugprone-forward-declaration-namespace)
class Path
{
public:
  explicit Path(PathVector* path_vector = nullptr);
  explicit Path(const PathGeometry& geometry, PathVector* path_vector = nullptr);
  explicit Path(const Path& path, PathVector* path_vector);
  explicit Path(std::vector<std::unique_ptr<Edge>> edges, PathVector* path_vector = nullptr);
  ~Path();
  Path(Path&&) = delete;
  Path& operator=(const Path&) = delete;
  Path& operator=(Path&&) = delete;

  static constexpr auto POINTS_POINTER = "points";

  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);

  Edge& add_edge(std::unique_ptr<Edge> edge);
  Edge& add_edge(std::shared_ptr<PathPoint> a, std::shared_ptr<PathPoint> b);
  std::shared_ptr<PathPoint> last_point() const;
  std::shared_ptr<PathPoint> first_point() const;

  /**
   * @brief remove removes the points specified by given `path_view` and returns the ownership
   *  of the touching edges.
   *  Inserts the given edge `bridge` to fill the gap and returns a pointer to `bridge`.
   * @param path_view specifies the points to remove
   * @param bridge Connects the two floating pathes.
   *  May be `nullptr`, in which case a new edge is created, if necessary.
   *  No bridge must be specified if no connection is required (because front, back or all points
   *  were removed).
   * @return ownership of the removed edges and a pointer to the new edge (or nullptr if no such
   *  edge was added).
   */
  std::pair<std::deque<std::unique_ptr<Edge>>, Edge*> remove(const PathView& path_view, std::unique_ptr<Edge> bridge = nullptr);

  /**
   * @brief Path::replace replaces the points selected by @param path_view with @param edges.
   * @param path_view the point selection to be removed
   * @param edges the edges that fill the gap.
   *  The first point of the first edge in this deque must match the last point left of the gap,
   *  unless there are no points left of the gap.
   *  The last point of the last edge in this deque must match the first point right of the gap,
   *  unless there are no points right of the gap.
   * @return The edges that have been removed.
   */
  std::deque<std::unique_ptr<Edge>> replace(const PathView& path_view, std::deque<std::unique_ptr<Edge>> edges);

  std::tuple<std::unique_ptr<Edge>, Edge*, Edge*> cut(Edge& edge, std::shared_ptr<PathPoint> p);
  [[nodiscard]] bool is_valid() const;
  [[nodiscard]] std::vector<PathPoint*> points() const;
  [[nodiscard]] std::vector<Edge*> edges() const;

  [[nodiscard]] PathPoint& at(std::size_t i) const;
  [[nodiscard]] bool contains(const PathPoint& point) const;
  [[nodiscard]] std::size_t find(const PathPoint& point) const;
  [[nodiscard]] std::shared_ptr<PathPoint> share(const PathPoint& point) const;
  PathGeometry geometry() const;

  [[nodiscard]] PathVector* path_vector() const;
  void set_path_vector(PathVector* path_vector);
  void set_interpolation(InterpolationMode interpolation) const;
  void set_single_point(std::shared_ptr<PathPoint> single_point);
  std::shared_ptr<PathPoint> extract_single_point();

  template<typename Edges> [[nodiscard]] static bool is_valid(const Edges& edges)
  {
    if (edges.empty()) {
      return true;
    }
    if (!std::all_of(edges.begin(), edges.end(), [](const auto& edge) { return edge->a() && edge->b(); })) {
      LERROR << "Is not valid because one or more edges contain invalid points.";
      return false;
    }
    for (auto it = begin(edges); next(it) != end(edges); advance(it, 1)) {
      if ((*it)->b() != (*next(it))->a()) {
        LERROR << "Is not valid because edges are not connected.";
        return false;
      }
    }
    return true;
  }

  QString print_edge_info() const;

private:
  std::shared_ptr<PathPoint> m_last_point;
  std::deque<std::unique_ptr<Edge>> m_edges;
  PathVector* m_path_vector;
  void set_last_point_from_edges();
};

}  // namespace
