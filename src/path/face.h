#pragma once

#include "geometry/vec2.h"
#include <QString>
#include <deque>
#include <list>
#include <memory>
#include <vector>

class QPainterPath;

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
class PathVectorView;

class Face
{
public:
  Face();
  Face(PathVectorView view);
  Face(const Face& other);
  Face(Face&& other) noexcept;
  Face& operator=(Face other);
  Face& operator=(Face&& other) noexcept;
  ~Face();
  friend void swap(Face& a, Face& b) noexcept;

  [[nodiscard]] double compute_aabb_area() const;
  [[nodiscard]] QString to_string() const;
  friend std::ostream& operator<<(std::ostream& os, const Face& face);
  [[nodiscard]] bool is_valid() const noexcept;
  [[nodiscard]] PathVectorView& path_vector_view();
  [[nodiscard]] const PathVectorView& path_vector_view() const;
  [[nodiscard]] double area() const;

  [[nodiscard]] bool operator==(const Face& other) const;
  [[nodiscard]] bool operator!=(const Face& other) const;
  [[nodiscard]] bool operator<(const Face& other) const;

  class ReferencePolisher;
  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);

private:
  std::unique_ptr<PathVectorView> m_path_vector_view;
};

enum class PolygonLocation { Inside, Outside, Edge };
[[nodiscard]] PolygonLocation polygon_contains(const std::vector<Vec2f>& polygon, const Vec2f& p);

}  // namespace omm
