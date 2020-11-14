#pragma once

#include <set>

#include "commands/command.h"
#include "common.h"
#include "scene/contextes.h"

namespace omm
{
class Object;
class Tag;
template<typename> class Tree;

template<typename StructureT> class RemoveCommand : public Command
{
public:
  // TODO is this the most elegant way to select a context type?
  using context_type = typename Contextes<typename StructureT::item_type>::Owning;
  using item_type = typename StructureT::item_type;
  RemoveCommand(StructureT& structure, const std::set<item_type*>& items);

  void undo() override;
  void redo() override;

private:
  std::vector<context_type> m_contextes;
  StructureT& m_structure;
};

}  // namespace omm
