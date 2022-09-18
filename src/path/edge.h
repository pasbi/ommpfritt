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
  [[nodiscard]] const std::shared_ptr<PathPoint>& a() const noexcept;
  [[nodiscard]] const std::shared_ptr<PathPoint>& b() const noexcept;
  [[nodiscard]] std::shared_ptr<PathPoint>& a() noexcept;
  [[nodiscard]] std::shared_ptr<PathPoint>& b() noexcept;
  [[nodiscard]] Path* path() const;
  [[nodiscard]] bool is_valid() const noexcept;
  [[nodiscard]] bool contains(const PathPoint* p) const noexcept;
  enum class Direction {Forward, Backward};
  [[nodiscard]] std::shared_ptr<PathPoint> start_point(const Direction& direction) const noexcept;
  [[nodiscard]] std::shared_ptr<PathPoint> end_point(const Direction& direction) const noexcept;

private:
  Path* m_path;
  std::shared_ptr<PathPoint> m_a = nullptr;
  std::shared_ptr<PathPoint> m_b = nullptr;
};

}  // namespace omm
