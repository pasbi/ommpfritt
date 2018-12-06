#pragma once

#include "commands/command.h"
#include "scene/contextes.h"

namespace omm
{

template<typename T>
class MoveCommand : public Command
{
public:
  using Context = typename Contextes<T>::Move;
  MoveCommand(Scene& scene, const std::vector<Context>& new_contextes);
  void redo();
  void undo();

private:
  std::vector<Context> m_new_contextes;
  std::vector<Context> m_old_contextes;
  Scene& m_scene;
};

}  // namespace omm
