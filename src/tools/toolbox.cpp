#include "tools/toolbox.h"
#include "common.h"
#include "scene/scene.h"
#include "tools/selectobjectstool.h"
#include "tools/selectpointstool.h"
#include "tools/selecttool.h"

namespace
{
auto make_tool_map(omm::Scene& scene)
{
  std::map<QString, std::unique_ptr<omm::Tool>> map;
  for (const auto& key : omm::Tool::keys()) {
    map.insert(std::pair(key, omm::Tool::make(key, scene)));
  }
  return map;
}

template<typename T> void remove_duplicates(std::list<T>& ls)
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

template<typename Map> auto collect_default_tools(Map&& tool_map)
{
  return std::map<omm::SceneMode, omm::Tool*>{
      {omm::SceneMode::Object, tool_map.at(omm::SelectObjectsTool::TYPE).get()},
      {omm::SceneMode::Vertex, tool_map.at(omm::SelectPointsTool::TYPE).get()},
  };
}

}  // namespace

namespace omm
{
ToolBox::ToolBox(Scene& scene)
    : m_tools(make_tool_map(scene)), m_default_tools(collect_default_tools(m_tools)), m_scene(scene)
{
  if (!m_tools.empty()) {
    m_active_tool = m_tools.begin()->second.get();
  }
}

Tool& ToolBox::active_tool() const
{
  return *m_active_tool;
}

void ToolBox::set_active_tool(const QString& key)
{
  return set_active_tool(m_tools.at(key).get());
}

void ToolBox::set_active_tool(Tool* tool)
{
  assert(tool != nullptr);
  if (m_active_tool != tool) {
    m_scene.set_mode(tool->scene_mode());
    if (m_active_tool != nullptr) {
      m_history.push_front(m_active_tool);
      ::remove_duplicates(m_history);
      m_active_tool->end();
    }
    m_active_tool = tool;
    m_scene.set_selection(std::set<AbstractPropertyOwner*>{m_active_tool});
    m_scene.set_mode(m_active_tool->scene_mode());
    m_active_tool->reset();
    Q_EMIT active_tool_changed(*m_active_tool);
  }
  m_active_tool->start();
}

void ToolBox::set_previous_tool()
{
  if (m_history.size() > 1) {
    auto *const name = *std::next(m_history.begin());
    set_active_tool(name);
  }
}

void ToolBox::set_scene_mode(SceneMode mode)
{
  if ((m_active_tool == nullptr) || m_active_tool->scene_mode() != mode) {
    set_active_tool(m_default_tools.at(mode));
  }
}

std::set<Tool*> ToolBox::tools() const
{
  return ::transform<Tool*, std::set>(m_tools, [](auto&& pair) { return pair.second.get(); });
}

}  // namespace omm
