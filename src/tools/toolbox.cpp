#include "tools/toolbox.h"
#include "scene/scene.h"

namespace
{
auto make_tool_map(omm::Scene& scene)
{
  std::map<std::string, std::unique_ptr<omm::Tool>> map;
  for (const auto& key : omm::Tool::keys()) {
    map.insert(std::pair(key, omm::Tool::make(key, scene)));
  }
  return map;
}

}  // namespace

namespace omm
{

ToolBox::ToolBox(Scene& scene)
  : m_tools(make_tool_map(scene))
  , m_scene(scene)
{
  m_active_tool = m_tools.begin()->second.get();
}

Tool& ToolBox::active_tool() const
{
  return *m_active_tool;
}

Tool& ToolBox::tool(const std::string& key) const
{
  try {
    return *m_tools.at(key);
  } catch (const std::out_of_range& e) {
    LOG(FATAL) << "Failed to load tool '" << key << "'.";
  }
}

void ToolBox::set_active_tool(const std::string &key)
{
  m_active_tool = m_tools.at(key).get();
  m_scene.set_selection(std::set<AbstractPropertyOwner*> { m_active_tool });
  m_active_tool->activate();
}

}  // namespace
