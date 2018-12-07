#pragma once

#include <memory>
#include "commands/command.h"

namespace omm
{

class Object;
template<typename> class Tree;

class AddObjectCommand : public Command
{
public:
  AddObjectCommand(Tree<Object>& structure, std::unique_ptr<Object> object);

  void undo() override;
  void redo() override;

private:
  std::unique_ptr<Object> m_owned;
  Object& m_reference;
  Tree<Object>& m_structure;
};

}  // namespace omm
