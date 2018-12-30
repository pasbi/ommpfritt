#pragma once

#include "tools/tool.h"

namespace omm
{

class ToolBox
{
public:
  ToolBox(Scene& scene);

  Tool& active_tool() const;

private:
  Tool* m_active_tool;
  const std::map<std::string, std::unique_ptr<Tool>> m_tools;

};

}  // namespace omm
