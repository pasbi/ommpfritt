#pragma once

#include <deque>
#include <set>
#include "common.h"

namespace omm
{

template<typename> class DisjointSetForest;
template<typename T> void swap(DisjointSetForest<T>& a, DisjointSetForest<T>& b) noexcept;

/**
 * @brief The DisjointSetForest class implements the disjoint set data structure.
 *  It does not follow the standard implementation because we typically want to lookup all
 *  members of a set.
 *  The classical find method only checks whether two items belong to the same set.
 */
template<typename T> class DisjointSetForest
{
public:
  using Joint = ::transparent_set<T>;
  DisjointSetForest(std::deque<Joint>&& forest = {})
    : m_forest(forest)
  {
  }

private:
  void join(const Joint& items_to_join, Joint& join_target)
  {
    for (auto it = m_forest.begin(); it != m_forest.end(); ++it) {
      if (!sets_disjoint(items_to_join, *it) && &*it != &join_target) {
        join_target.insert(it->begin(), it->end());
        it->clear();
      }
    }
    std::erase_if(m_forest, [](const auto& set) { return set.empty(); });
  }

public:
  static bool sets_disjoint(const Joint& a, const Joint& b)
  {
    return a.end() == std::find_first_of(a.begin(), a.end(), b.begin(), b.end());
  }

  /**
   * @brief insert insert set into the forest.
   *  If any item of set is already known, the sets are joined accordingly.
   *  Examples:
   *  - insert {A, B} into ()                  -> ({A, B})
   *  - insert {C, D} into ({A, B})            -> ({A, B}, {C, D})
   *  - insert {B, E} into ({A, B}, {C, D})    -> ({A, B, E}, {C, D})
   *  - insert {A, C} into ({A, B}, {C, D, E}) -> ({A, B, C, D, E})
   * @return The set from the forest that includes the given set
   */
  Joint insert(const Joint& set)
  {
    for (auto it = m_forest.begin(); it != m_forest.end(); ++it) {
      if (!sets_disjoint(set, *it)) {
        it->insert(set.begin(), set.end());
        join(set, *it);
        return *it;
      }
    }
    m_forest.push_back(set);
    return set;
  }

  /**
   * @brief get returns the set that contains `key` or the empty set if there is no such.
   */
  template<typename K> Joint get(const K& key) const
  {
    for (const auto& set : m_forest) {
      if (set.contains(key)) {
        return set;
      }
    }
    return {};
  }

  /**
   * @brief remove removes all items in `set` from the forest.
   */
  void remove(const Joint& set)
  {
    const auto overlap = [&set](const auto& other_set) { return !sets_disjoint(other_set, set); };
    const auto it = std::remove_if(m_forest.begin(), m_forest.end(), overlap);
    m_forest.erase(it, m_forest.end());
  }

  friend void swap<>(DisjointSetForest<T>& a, DisjointSetForest<T>& b) noexcept;

  const std::deque<Joint>& sets() const { return m_forest; }

protected:
  std::deque<Joint> m_forest;
  void remove_empty_sets()
  {
    m_forest.erase(std::remove_if(m_forest.begin(), m_forest.end(), [](const auto& set) {
      return set.empty();
    }), m_forest.end());
  }
};

template<typename T> void swap(DisjointSetForest<T>& a, DisjointSetForest<T>& b) noexcept
{
  swap(a.m_forest, b.m_forest);
}

}  // namespace omm
