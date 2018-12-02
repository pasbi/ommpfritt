#pragma once

#include "commands/command.h"
#include "renderers/style.h"

namespace omm
{

class Scene;
class NewStyleCommand : public Command
{

public:
  NewStyleCommand(Scene& scene, std::unique_ptr<Style> style);
  void undo();
  void redo();

private:
  Scene& m_scene;
  std::unique_ptr<Style> m_owned;
  Style& m_reference;
};

}  // namespace omm