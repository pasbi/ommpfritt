#pragma once

#include <deque>
#include <set>
#include <vector>


namespace omm
{

class PathVector;
class PathPoint;


/**
 * @brief The PathVectorIsomorphism struct holds pointers to the points of a number of PathVectors
 * if they are isomorph, i.e., have the same structure.
 * That is, all PathVectors must have the same number of path and each `ith` path from any PathVector
 * has the same number of points.
 * The number of paths in each path vector is stored in `n_paths`.
 * The number of points in each `ith` path from any PathVector is stored in `n_points[i]`.
 */
class PathVectorIsomorphism
{
public:
  explicit PathVectorIsomorphism(const std::deque<PathVector>& path_vectors);
  using Points = std::deque<std::deque<std::vector<PathPoint*>>>;
  const Points& points() const;
  std::size_t n_paths() const;
  std::size_t n_points(std::size_t path_index) const;
  bool is_valid() const;
  std::set<std::vector<PathPoint*>> correspondences() const;

private:
  Points m_points;

  /**
   * @brief n_paths number of paths in each path vector
   */
  std::size_t m_n_paths;

  /**
   * @brief n_points number of points in each ith path
   */
  std::deque<std::size_t> m_n_points;
  bool m_is_valid = false;
};

}  // namespace omm
