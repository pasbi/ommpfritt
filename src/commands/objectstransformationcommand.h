#pragma once

#include "commands/command.h"
#include "geometry/objecttransformation.h"
#include <map>
#include <set>

namespace omm
{
class Object;

class ObjectsTransformationCommand : public Command
{
public:
  enum class TransformationMode { Object, Axis };
  using Map = std::map<Object*, ObjectTransformation>;
  ObjectsTransformationCommand(const Map& transformations, TransformationMode t_mode);
  void undo() override;
  void redo() override;
  [[nodiscard]] bool is_noop() const override;
  bool mergeWith(const QUndoCommand* command) override;
  [[nodiscard]] int id() const override;

private:
  Map m_old_transformations, m_new_transformations;
  const TransformationMode m_transformation_mode;
  void apply(const Map& map);
  [[nodiscard]] std::set<Object*> affected_objects() const;
};

}  // namespace omm
