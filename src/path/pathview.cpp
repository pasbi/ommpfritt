#include "path/pathview.h"
#include <tuple>
#include <ostream>
#include "path/path.h"

namespace omm
{

PathView::PathView(Path& path, std::size_t begin, std::size_t size)
  : m_path(&path), m_begin(begin), m_size(size)
{
}

Path& PathView::path() const
{
  return *m_path;
}

std::size_t PathView::begin() const
{
  return m_begin;
}

std::size_t PathView::end() const
{
  return m_begin + m_size;
}

std::size_t PathView::size() const
{
  return m_size;
}

bool operator<(const PathView& a, const PathView& b)
{
  static constexpr auto as_tuple = [](const PathView& a) {
    return std::tuple{&a.path(), a.begin(), a.size()};
  };
  // NOLINTNEXTLINE(modernize-use-nullptr)
  return as_tuple(a) < as_tuple(b);
}

std::ostream& operator<<(std::ostream& ostream, const PathView& path_view)
{
  ostream << "Path[" << &path_view.path() << " " << path_view.begin() << " " << path_view.size() << "]";
  return ostream;
}

}  // namespace omm
