#pragma once

#include <QUndoCommand>

namespace omm
{

class Project;
class Scene;

class Command : public QUndoCommand
{
protected:
  Command(Project& scene, const std::string& label);
  Project& m_project;
  Scene& scene() const;
};

}  // namespace omm
