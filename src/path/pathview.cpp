#include "path/pathview.h"
#include <tuple>
#include <ostream>
#include "path/path.h"

namespace omm
{

PathView::PathView(Path& path, std::size_t index, std::size_t size)
  : path(&path), index(index), size(size)
{
}

std::deque<PathPoint*> PathView::points() const
{
  auto points = path->points();
  points.erase(points.begin(), std::next(points.begin(), index));
  points.erase(std::next(points.begin(), size), points.end());
  return points;
}

bool operator<(const PathView& a, const PathView& b)
{
  static constexpr auto as_tuple = [](const PathView& a) {
    return std::tuple{a.path, a.index};
  };
  // NOLINTNEXTLINE(modernize-use-nullptr)
  return as_tuple(a) < as_tuple(b);
}

std::ostream& operator<<(std::ostream& ostream, const PathView& path_view)
{
  ostream << "Path[" << path_view.path << " " << path_view.index << " " << path_view.size << "]";
  return ostream;
}

}  // namespace omm
