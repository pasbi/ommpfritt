#pragma once

#include "commands/command.h"
#include "renderers/style.h"

namespace omm
{

class StylePool;
class NewStyleCommand : public Command
{

public:
  NewStyleCommand(StylePool& style_pool, std::unique_ptr<Style> style);
  void undo();
  void redo();

private:
  StylePool& m_style_pool;
  std::unique_ptr<Style> m_owned;
  Style& m_reference;
};

}  // namespace omm