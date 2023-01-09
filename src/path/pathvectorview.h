#pragma once

#include "geometry/vec2.h"
#include "path/dedge.h"
#include <2geom/path.h>
#include <deque>
#include <vector>

class QPainterPath;
class QRectF;

namespace omm
{

class Edge;
class PathPoint;

class PathVectorView
{
public:
  PathVectorView() = default;
  explicit PathVectorView(std::deque<DEdge> edges);

  /**
   * @brief is_simply_closed returns true if every two consecutive edge pairs (including last and
   *   first) have exactly one point in common.
   *   cases:
   *   - zero edges: return false
   *   - one edge: return true iff the edge is a loop, i.e., if both of its points are the same.
   *   - two edges: return true iff the two edges have two points in common.
   */
  [[nodiscard]] bool is_simply_closed() const;
  [[nodiscard]] const std::deque<DEdge>& edges() const;
  [[nodiscard]] QPainterPath to_painter_path() const;
  [[nodiscard]] Geom::Path to_geom() const;
  [[nodiscard]] bool contains(const Vec2f& pos) const;
  [[nodiscard]] QString to_string() const;
  [[nodiscard]] std::vector<PathPoint*> path_points() const;
  [[nodiscard]] QRectF bounding_box() const;
  [[nodiscard]] std::vector<Vec2f> bounding_polygon() const;

  /**
   * @brief normalize PathVectorViews are defined up to
   *  - the direction (m_edges can be reveresed and it still describes the same view
   *      because the actual direction is given by the paths the edges belong to).
   *  - the first edge if it is closed (m_edges can be rotated without chaning the view)
   *  This function returns the edges of the PathVectorView in normalized order.
   */
  std::vector<Edge*> normalized() const;

private:
  std::deque<DEdge> m_edges;
};

[[nodiscard]] bool operator==(const PathVectorView& a, const PathVectorView& b);
[[nodiscard]] bool operator<(const PathVectorView& a, const PathVectorView& b);

}  // namespace omm
