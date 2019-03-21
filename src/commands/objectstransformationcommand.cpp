#include "commands/objectstransformationcommand.h"
#include "common.h"
#include "objects/object.h"

namespace
{

auto
make_alternatives(std::set<omm::Object*> objects, const omm::ObjectTransformation::Mat& t)
{
  omm::Object::remove_internal_children(objects);
  std::map<omm::Object*, omm::ObjectTransformation> alternatives;
  for (const auto& object : objects) {
    const auto new_t = omm::ObjectTransformation(t * object->global_transformation().to_mat());
    alternatives.insert(std::make_pair(object, new_t));
  }
  return alternatives;
}

void swap_transformation(omm::Object& object, omm::ObjectTransformation& transformation)
{
  const auto old_transformation = object.global_transformation();
  object.set_global_transformation(transformation);
  transformation = old_transformation;
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
::ObjectsTransformationCommand(const std::set<Object*>& objects, const ObjectTransformation& t)
  : Command(QObject::tr("ObjectsTransformationCommand").toStdString())
  , m_alternative_transformations(make_alternatives(objects, t.to_mat()))
{
}

void ObjectsTransformationCommand::undo()
{
  for (const auto& p : m_alternative_transformations) {
    swap_transformation(*p.first, m_alternative_transformations[p.first]);
  }
}

void ObjectsTransformationCommand::redo()
{
  for (const auto& p : m_alternative_transformations) {
    swap_transformation(*p.first, m_alternative_transformations[p.first]);
  }
}

int ObjectsTransformationCommand::id() const
{
  return OBJECTS_TRANSFORMATION_COMMAND_ID;
}

bool ObjectsTransformationCommand::mergeWith(const QUndoCommand* command)
{
  // merging happens automatically!
  const auto& ot_command = static_cast<const ObjectsTransformationCommand&>(*command);
  return has_same_objects(ot_command.m_alternative_transformations, m_alternative_transformations);
}

}  // namespace omm
