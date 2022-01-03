#pragma once

#include <cstdint>
#include <iosfwd>
#include <deque>

namespace omm
{

class Path;
class PathPoint;

struct PathView
{
public:
  explicit PathView(Path& path, std::size_t index, std::size_t size);
  friend bool operator<(const PathView& a, const PathView& b);
  friend std::ostream& operator<<(std::ostream& ostream, const PathView& path_view);
  [[nodiscard]] std::deque<PathPoint*> points() const;
  Path* path;
  std::size_t index;
  std::size_t size;
};

}  // namepsace
