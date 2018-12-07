#pragma once

#include <set>
#include "scene/contextes.h"
#include "commands/command.h"

namespace omm
{

class Style;
template<typename> class List;

class RemoveStylesCommand : public Command
{
public:
  RemoveStylesCommand(List<Style>& structure, const std::set<omm::Style*>& styles);

  void undo() override;
  void redo() override;

private:
  std::vector<StyleListOwningContext> m_contextes;
  List<Style>& m_structure;
};

}  // namespace omm
