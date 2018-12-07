#pragma once

#include <set>

#include "commands/command.h"
#include "common.h"
#include "scene/contextes.h"

namespace omm
{

class Object;
template<typename> class Tree;

class RemoveObjectsCommand : public Command
{
public:
  RemoveObjectsCommand(Tree<Object>& structure, const std::set<omm::Object*>& objects);

  void undo() override;
  void redo() override;

private:
  std::vector<ObjectTreeOwningContext> m_contextes;
  Tree<Object>& m_structure;
};

}  // namespace
