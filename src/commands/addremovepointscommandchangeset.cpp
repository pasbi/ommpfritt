#include "commands/addremovepointscommandchangeset.h"

#include "path/edge.h"
#include "path/path.h"
#include "transform.h"

namespace omm
{

AddRemovePointsCommandChangeSet::AddRemovePointsCommandChangeSet(const PathView& view,
                                                                 std::deque<std::unique_ptr<Edge>> edges,
                                                                 std::shared_ptr<PathPoint> single_point)
  : m_view(view), m_owned_edges(std::move(edges)), m_owned_point(std::move(single_point))
{
  assert(Path::is_valid(m_owned_edges));
  assert((m_owned_point == nullptr) || m_owned_edges.empty());
}

void AddRemovePointsCommandChangeSet::swap()
{
  std::size_t added_point_count = 0;

  if (m_view.path().points().empty() && m_owned_point) {
    // path empty, add single point
    assert(m_owned_edges.empty());
    m_view.path().set_single_point(std::move(m_owned_point));
    added_point_count = 1;
  } else if (m_view.path().points().size() == 1 && m_view.point_count() == 1) {
    // path contains only a single point which is going to be removed
    m_owned_point = m_view.path().extract_single_point();
    added_point_count = 0;
  } else {
    // all other cases are handled by Path::replace
    auto& path = m_view.path();
    if (m_owned_edges.empty()) {
      added_point_count = 0;
    } else if (path.points().empty()) {
      added_point_count = m_owned_edges.size() + 1;
    } else if (m_view.begin() == 0 || m_view.end() == path.points().size()) {
      added_point_count = m_owned_edges.size();
    } else {
      added_point_count = m_owned_edges.size() - 1;
    }
    m_owned_edges = path.replace(m_view, std::move(m_owned_edges));
  }

  m_view = PathView{m_view.path(), m_view.begin(), added_point_count};
}

std::vector<Edge*> AddRemovePointsCommandChangeSet::owned_edges() const
{
  return util::transform<std::vector>(m_owned_edges, &std::unique_ptr<Edge>::get);
}

}  // namespace omm
