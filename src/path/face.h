#pragma once

#include <QString>
#include <list>
#include <deque>
#include <memory>

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
  [[nodiscard]] bool is_valid() const noexcept;
  [[nodiscard]] PathVectorView& path_vector_view();
  [[nodiscard]] const PathVectorView& path_vector_view() const;
  [[nodiscard]] bool contains(const Face& other) const;

  [[nodiscard]] bool operator==(const Face& other) const;
  [[nodiscard]] bool operator!=(const Face& other) const;
  [[nodiscard]] bool operator<(const Face& other) const;
  void normalize();

  class ReferencePolisher;
  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);

private:
  std::unique_ptr<PathVectorView> m_path_vector_view;
};

}  // namespace omm
