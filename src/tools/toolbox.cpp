#include "tools/toolbox.h"

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
{
  m_active_tool = m_tools.at("ScaleTool").get();
}

Tool& ToolBox::active_tool() const
{
  return *m_active_tool;
}

}  // namespace
