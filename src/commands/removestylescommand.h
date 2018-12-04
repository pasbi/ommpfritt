#pragma once

#include <set>
#include "scene/listcontext.h"
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
  std::vector<OwningListContext<Style>> m_contextes;
  Scene& m_scene;
};

}  // namespace omm
