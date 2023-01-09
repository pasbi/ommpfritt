#pragma once

#include <cstdint>
#include <iosfwd>
#include <deque>

namespace omm
{

class Path;
class PathPoint;

class PathView
{
public:
  explicit PathView(Path& path, std::size_t begin, std::size_t point_count);
  friend bool operator<(const PathView& a, const PathView& b);
  friend std::ostream& operator<<(std::ostream& ostream, const PathView& path_view);
  [[nodiscard]] std::deque<PathPoint*> points() const;
  [[nodiscard]] Path& path() const;
  [[nodiscard]] std::size_t begin() const;
  [[nodiscard]] std::size_t end() const;
  [[nodiscard]] std::size_t point_count() const ;

private:
  Path* m_path;
  std::size_t m_begin;
  std::size_t m_point_count;
};

}  // namepsace
