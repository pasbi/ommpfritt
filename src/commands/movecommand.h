#pragma once

#include "commands/command.h"
#include "scene/stylelist.h"
#include "scene/contextes.h"

namespace omm
{
template<typename StructureT> class MoveCommand : public Command
{
public:
  using context_type = typename Contextes<typename StructureT::item_type>::Move;
  MoveCommand(StructureT& structure, const std::deque<context_type>& new_contextes);
  void redo() override;
  void undo() override;

private:
  std::deque<context_type> m_old_contextes;
  std::deque<context_type> m_new_contextes;
  StructureT& m_structure;
};

}  // namespace omm
