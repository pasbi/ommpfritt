#pragma once

#include "tools/tool.h"
#include "objects/object.h"
#include "tools/handles/pathhandle.h"

namespace omm
{

class Path;

class PathTool : public Tool
{
public:
  using Tool::Tool;
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "PathTool";
  // void draw(AbstractRenderer& renderer) override;

private:
  std::set<Path*> m_paths;
};

}  // namespace omm
