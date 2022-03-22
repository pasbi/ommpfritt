#pragma once

#include <set>

namespace omm
{

class Object;

/**
 * @brief The TopLevelSplit class splits a given set of objects into top level objects and non top
 *  level object.
 *  An object is a top level object if and only if it has no parent in the given set.
 *  That is, for one scene, multiple objects may be considererd top level (e.g., siblings) although
 *  all of them have a parent in the scene if they don't have a parent in the given set.
 */
class TopLevelSplit
{
public:
  TopLevelSplit(std::set<Object*> objects);
  [[nodiscard]] const std::set<Object*>& non_top_level_objects() const;
  [[nodiscard]] const std::set<Object*>& top_level_objects() const;

private:
  std::set<Object*> m_non_top_level_objects;
  std::set<Object*> m_top_level_objects;
};

}  // namespace omm
