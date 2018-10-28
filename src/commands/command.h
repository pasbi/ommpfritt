#pragma once

#include <QUndoCommand>

namespace omm
{

class Project;
class Scene;

class Command : public QUndoCommand
{
protected:
  Command(Project& scene, const QString& label);
  Project& m_project;
  Scene& scene();
};

}  // namespace omm
