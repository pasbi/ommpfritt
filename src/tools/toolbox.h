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
  Tool& tool(const std::string& key) const;
  void set_active_tool(const std::string& key);
  void set_previous_tool();
  void switch_between_object_and_point_selection();

Q_SIGNALS:
  void active_tool_changed(const Tool& active_tool);

private:
  Tool* m_active_tool;
  const std::map<std::string, std::unique_ptr<Tool>> m_tools;
  Scene& m_scene;
  std::list<std::string> m_history;
};

}  // namespace omm
