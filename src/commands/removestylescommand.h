#pragma once

#include <set>
#include "scene/contextes.h"
#include "commands/command.h"

namespace omm
{

class Style;

class RemoveStylesCommand : public Command
{
public:
  RemoveStylesCommand(Scene& scene, const std::set<omm::Style*>& styles);

  void undo() override;
  void redo() override;

private:
  std::vector<StyleListOwningContext> m_contextes;
  Scene& m_scene;
};

}  // namespace omm
