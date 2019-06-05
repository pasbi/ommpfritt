#include "tools/toolbox.h"
#include "scene/scene.h"
#include "tools/selecttool.h"
#include "tools/selectpointstool.h"
#include "tools/selectobjectstool.h"

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

template<typename T> void unique(std::list<T>& ls)
{
  std::set<T> occurences;
  ls.remove_if([&occurences](const T& item) {
    if (::contains(occurences, item)) {
      return true;
    } else {
      occurences.insert(item);
      return false;
    }
  });
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
  } catch (const std::out_of_range&) {
    LFATAL("Failed to load tool '%s'.", key.c_str());
  }
}

void ToolBox::set_active_tool(const std::string &key)
{
  m_history.push_front(key);
  ::unique(m_history);
  if (m_active_tool) {
    m_active_tool->end();
  }
  m_active_tool = m_tools.at(key).get();
  m_scene.set_selection(std::set<AbstractPropertyOwner*> { m_active_tool });
  m_active_tool->on_scene_changed();
}

void ToolBox::set_previous_tool()
{
  if (m_history.size() > 1) {
    const auto name = *std::next(m_history.begin());
    set_active_tool(name);
  }
}

void ToolBox::switch_between_object_and_point_selection()
{
  if (m_active_tool->type() == SelectPointsTool::TYPE) {
    set_active_tool(SelectObjectsTool::TYPE);
  } else if (m_active_tool->type() == SelectObjectsTool::TYPE) {
    set_active_tool(SelectPointsTool::TYPE);
  } else {
    for (auto&& tool : m_history) {
      if (tool == SelectPointsTool::TYPE) {
        set_active_tool(SelectPointsTool::TYPE);
        return;
      } else if (tool == SelectObjectsTool::TYPE) {
        set_active_tool(SelectObjectsTool::TYPE);
        return;
      }
    }
    set_active_tool(SelectObjectsTool::TYPE);
  }
  return;
}

}  // namespace
