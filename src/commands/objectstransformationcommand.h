#pragma once

#include <set>
#include "commands/command.h"
#include "geometry/objecttransformation.h"

namespace omm
{

class Object;
class ObjectsTransformationCommand : public Command
{
public:
  ObjectsTransformationCommand(const std::set<Object*>& objects, const ObjectTransformation& t);
  void undo() override;
  void redo() override;
  int id() const override;
  bool mergeWith(const QUndoCommand* command) override;

private:
  std::map<Object*, ObjectTransformation> m_alternative_transformations;
};

}  // namespace omm
