#pragma once

#include "commands/command.h"
#include "scene/contextes.h"

namespace omm
{
template<typename Structure> class CopyCommand : public Command
{
public:
  using Context = typename Contextes<typename Structure::item_type>::Owning;
  CopyCommand(Structure& structure, std::deque<Context> contextes);

  void undo() override;
  void redo() override;

private:
  std::deque<Context> m_contextes;
  Structure& m_structure;
};

}  // namespace omm
