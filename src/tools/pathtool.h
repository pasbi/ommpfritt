#pragma once

#include "tools/tool.h"
#include "objects/object.h"

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
  void draw(AbstractRenderer& renderer) const override;
  std::set<Path*> paths() const;
  void set_selection(const std::set<Object*>& objects) override;
};

}  // namespace omm
