#pragma once

#include "tools/tool.h"
#include <list>

namespace omm
{

class Scene;

class ToolBox : public QObject
{
  Q_OBJECT
public:
  ToolBox(Scene& scene);

  Tool& active_tool() const;
  Tool& tool(const QString& key) const;
  void set_active_tool(const QString& key);
  void set_active_tool(Tool* tool);
  void set_previous_tool();
  void set_scene_mode(SceneMode mode);

Q_SIGNALS:
  void active_tool_changed(const Tool& active_tool);

private:
  Tool* m_active_tool = nullptr;
  const std::map<QString, std::unique_ptr<Tool>> m_tools;
  const std::map<SceneMode, Tool*> m_default_tools;
  Scene& m_scene;
  std::list<Tool*> m_history;
};

}  // namespace omm
