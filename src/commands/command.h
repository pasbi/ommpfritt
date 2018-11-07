#pragma once

#include <QUndoCommand>

namespace omm
{

class Project;
class Scene;

class Command : public QUndoCommand
{
protected:
  Command(Scene& scene, const std::string& label);

public:
  Scene& scene;
};

}  // namespace omm
