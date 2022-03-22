#include "scene/toplevelsplit.h"
#include "objects/object.h"

namespace omm
{

TopLevelSplit::TopLevelSplit(std::set<Object*> objects)
{
  const auto all = objects;
  m_top_level_objects = std::move(objects);  // NOLINT(cppcoreguidelines-prefer-member-initializer)
  Object::remove_internal_children(m_top_level_objects);
  std::set_difference(all.begin(),
                      all.end(),
                      m_top_level_objects.begin(),
                      m_top_level_objects.end(),
                      std::inserter(m_non_top_level_objects, m_non_top_level_objects.end()));
}

const std::set<Object*>& TopLevelSplit::non_top_level_objects() const
{
  return m_non_top_level_objects;
}

const std::set<Object*>& TopLevelSplit::top_level_objects() const
{
  return m_top_level_objects;
}

}  // namespace omm
