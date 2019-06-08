#include "commands/objectstransformationcommand.h"
#include "geometry/matrix.h"
#include "common.h"
#include "objects/object.h"

namespace
{

auto
make_alternatives(std::set<omm::Object*> objects, const omm::Matrix& t)
{
  omm::Object::remove_internal_children(objects);
  std::map<omm::Object*, omm::ObjectTransformation> alternatives;
  for (const auto& object : objects) {
    alternatives.insert(std::make_pair(object, t * object->global_transformation(false).to_mat()));
  }
  return alternatives;
}

template<typename MapT>
bool has_same_objects(const MapT& a, const MapT& b)
{
  if (a.size() != b.size()) {
    return false;
  } else {
    for (const auto& p : a) {
      if (b.count(p.first) == 0) {
        return false;
      }
    }
    return true;
  }
}

}  // namespace

namespace omm
{

ObjectsTransformationCommand
::ObjectsTransformationCommand(const std::set<Object*>& objects, const Matrix& t,
                               const TransformationMode transformation_mode)
  : Command(QObject::tr("ObjectsTransformationCommand").toStdString())
  , m_alternative_transformations(make_alternatives(objects, t))
  , m_transformation_mode(transformation_mode)
{
}

void ObjectsTransformationCommand::undo()
{
  for (auto& [object, alternative_transformation] : m_alternative_transformations) {
    const auto old_transformation = object->global_transformation(true);
    switch (m_transformation_mode) {
    case TransformationMode::Axis:
      object->set_global_axis_transformation(alternative_transformation, true);
      break;
    case TransformationMode::Object:
      object->set_global_transformation(alternative_transformation, true);
      break;
    }
    alternative_transformation = old_transformation;
  }
}

void ObjectsTransformationCommand::redo() { undo(); }
int ObjectsTransformationCommand::id() const { return OBJECTS_TRANSFORMATION_COMMAND_ID; }

bool ObjectsTransformationCommand::mergeWith(const QUndoCommand* command)
{
  // merging happens automatically!
  const auto& ot_command = static_cast<const ObjectsTransformationCommand&>(*command);
  return has_same_objects(ot_command.m_alternative_transformations, m_alternative_transformations);
}

bool ObjectsTransformationCommand::is_noop() const
{
  return std::all_of(m_alternative_transformations.begin(), m_alternative_transformations.end(),
                     [](const auto& ot) { return ot.second.is_identity(); });
}

}  // namespace omm
