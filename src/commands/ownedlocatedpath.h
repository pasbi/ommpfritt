#pragma once

#include <memory>
#include <deque>

namespace omm
{

class Edge;
class Path;
class PathPoint;

class OwnedLocatedPath
{
public:
  explicit OwnedLocatedPath(Path* path, std::size_t point_offset, std::deque<std::shared_ptr<PathPoint>> points);
  ~OwnedLocatedPath();
  OwnedLocatedPath(OwnedLocatedPath&& other) = default;
  OwnedLocatedPath& operator=(OwnedLocatedPath&& other) = default;
  OwnedLocatedPath(const OwnedLocatedPath& other) = delete;
  OwnedLocatedPath& operator=(const OwnedLocatedPath& other) = delete;
  friend bool operator<(const OwnedLocatedPath& a, const OwnedLocatedPath& b);
  std::deque<std::unique_ptr<Edge>> create_edges() const;
  std::shared_ptr<PathPoint> single_point() const;
  std::size_t point_offset() const;
  Path* path() const;

private:
  Path* m_path = nullptr;
  std::size_t m_point_offset;
  std::deque<std::shared_ptr<PathPoint>> m_points;
};

}  // namespace omm
