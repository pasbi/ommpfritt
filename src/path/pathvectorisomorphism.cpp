#include "path/pathvectorisomorphism.h"
#include "path/path.h"
#include "path/pathvector.h"
#include "transform.h"


namespace omm
{

PathVectorIsomorphism::PathVectorIsomorphism(const std::deque<PathVector*>& path_vectors)
{
  m_points = util::transform(path_vectors, [](const auto* const pv) {
    return util::transform(pv->paths(), [](const auto* const path) { return path->points(); });
  });

  const auto set = util::transform<std::set>(m_points, [](const auto& pv) { return pv.size(); });
  if (set.size() != 1) {
    // Not all PathVectors have the same number of paths
    return;
  }
  m_n_paths = *set.begin();

  m_n_paths = m_points.front().size();
  for (std::size_t i = 0; i < m_n_paths; ++i) {
    const auto set = util::transform<std::set>(m_points, [i](const auto& pv) { return pv.at(i).size(); });
    if (set.size() != 1) {
      // Not all ith paths of each PathVector have same number of points
      return;
    }
    m_n_points.push_back(*set.begin());
  }
  m_is_valid = true;
}

const PathVectorIsomorphism::Points& PathVectorIsomorphism::points() const
{
  return m_points;
}

std::size_t PathVectorIsomorphism::n_paths() const
{
  return m_n_paths;
}

std::size_t PathVectorIsomorphism::n_points(std::size_t path_index) const
{
  return m_n_points.at(path_index);
}

bool PathVectorIsomorphism::is_valid() const
{
  return m_is_valid;
}

std::set<std::vector<PathPoint*>> PathVectorIsomorphism::correspondences() const
{
  std::set<std::vector<PathPoint*>> set;
  const auto n_path_vectors = m_points.size();
  for (std::size_t i = 0; i < m_n_paths; ++i) {
    for (std::size_t j = 0; j < m_n_points.at(i); ++j) {
      std::vector<PathPoint*> ps;
      ps.reserve(n_path_vectors);
      for (std::size_t k = 0; k < n_path_vectors; ++k) {
        ps.push_back(m_points.at(k).at(i).at(j));
      }
      set.insert(ps);
    }
  }
  return set;
}

}  // namespace omm
