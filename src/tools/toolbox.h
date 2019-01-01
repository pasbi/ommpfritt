#pragma once

#include "tools/tool.h"

namespace omm
{

class Scene;

class ToolBox
{
public:
  ToolBox(Scene& scene);

  Tool& active_tool() const;
  Tool& tool(const std::string& key) const;
  void set_active_tool(const std::string& key);

private:
  Tool* m_active_tool;
  const std::map<std::string, std::unique_ptr<Tool>> m_tools;
  Scene& m_scene;

};

}  // namespace omm
