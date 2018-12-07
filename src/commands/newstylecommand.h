#pragma once

#include "commands/command.h"
#include "renderers/style.h"

namespace omm
{

class Style;
template<typename> class List;

class NewStyleCommand : public Command
{

public:
  NewStyleCommand(List<Style>& structure, std::unique_ptr<Style> style);
  void undo();
  void redo();

private:
  List<Style>& m_structure;
  std::unique_ptr<Style> m_owned;
  Style& m_reference;
};

}  // namespace omm