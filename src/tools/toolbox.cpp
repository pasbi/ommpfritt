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
    if (occurences.contains(item)) {
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

const decltype(ToolBox::m_default_tools) ToolBox::m_default_tools {
  {omm::SceneMode::Object, omm::SelectObjectsTool::TYPE},
  {omm::SceneMode::Vertex, omm::SelectPointsTool::TYPE},
};

ToolBox::ToolBox(Scene& scene)
    : m_tools(make_tool_map(scene))
    , m_scene(scene)
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
  return set_active_tool(*m_tools.at(key));
}

void ToolBox::set_active_tool(Tool& tool)
{
  if (m_active_tool != &tool) {
    m_scene.set_mode(tool.scene_mode());
    if (m_active_tool != nullptr) {
      m_history.push_front(m_active_tool);
      ::remove_duplicates(m_history);
      m_active_tool->end();
    }
    m_active_tool = &tool;
    m_scene.set_selection(std::set<AbstractPropertyOwner*>{m_active_tool});
    m_scene.set_mode(m_active_tool->scene_mode());
    m_active_tool->reset();
    Q_EMIT active_tool_changed(*m_active_tool);
  }
  m_active_tool->start();
}

void ToolBox::activate_previous_tool()
{
  if (m_history.size() > 1) {
    auto& name = **std::next(m_history.begin());
    set_active_tool(name);
  }
}

void ToolBox::set_scene_mode(SceneMode mode)
{
  if ((m_active_tool == nullptr) || m_active_tool->scene_mode() != mode) {
    set_active_tool(m_default_tools.at(mode));
  }
}

::transparent_set<Tool*> ToolBox::tools() const
{
  return util::transform<::transparent_set>(m_tools, [](auto&& pair) { return pair.second.get(); });
}

void ToolBox::activate_default_tool()
{
  set_active_tool(m_default_tools.at(m_scene.current_mode()));
}

}  // namespace omm
