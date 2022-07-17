#pragma once

#include <QString>
#include <memory>

namespace omm
{

class Path;
class PathPoint;
class Point;

class Edge
{
public:
  Edge() = default;
  explicit Edge(std::shared_ptr<PathPoint> a, std::shared_ptr<PathPoint> b, Path* path);
  Edge(const Edge&) = delete;
  Edge(Edge&&) = default;
  Edge& operator=(const Edge&) = delete;
  Edge& operator=(Edge&&) = default;
  ~Edge() = default;
  [[nodiscard]] QString label() const;
  void flip() noexcept;
  [[nodiscard]] bool has_point(const PathPoint* p) noexcept;
  [[nodiscard]] std::shared_ptr<PathPoint> a() const noexcept;
  [[nodiscard]] std::shared_ptr<PathPoint> b() const noexcept;
  [[nodiscard]] bool operator<(const Edge& other) const noexcept;
  Path* path() const;

private:
  Path* m_path;
  std::shared_ptr<PathPoint> m_a = nullptr;
  std::shared_ptr<PathPoint> m_b = nullptr;
};

}  // namespace omm
