#pragma once

#include "commands/command.h"
#include "scene/contextes.h"

namespace omm
{

template<typename T>
class CopyCommand : public Command
{
public:
  using Context = typename Contextes<T>::Owning;
  CopyCommand(Scene& scene, std::vector<Context> contextes);

  void undo() override;
  void redo() override;

private:
  std::vector<Context> m_contextes;
  Scene& m_scene;
};

}  // namespace
