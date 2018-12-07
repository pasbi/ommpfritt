#pragma once

#include "commands/command.h"
#include "scene/contextes.h"

namespace omm
{

template<typename Structure>
class MoveCommand : public Command
{
public:
  using Context = typename Contextes<typename Structure::item_type>::Move;
  MoveCommand(Structure& structure, const std::vector<Context>& new_contextes);
  void redo();
  void undo();

private:
  std::vector<Context> m_new_contextes;
  std::vector<Context> m_old_contextes;
  Structure& m_structure;
};

}  // namespace omm
