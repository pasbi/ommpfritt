#include "commands/objectstransformationcommand.h"
#include "common.h"
#include "objects/object.h"

namespace
{
auto get_old_transformations(std::set<omm::Object*> objects)
{
  omm::Object::remove_internal_children(objects);
  omm::ObjectsTransformationCommand::Map map;
  for (auto&& object : objects) {
    map.insert(std::pair(object, object->global_transformation(omm::Space::Scene)));
  }
  return map;
}

auto get_new_transformations(const omm::ObjectsTransformationCommand::Map& new_transformations)
{
  auto objects = get_keys(new_transformations);
  omm::Object::remove_internal_children(objects);
  omm::ObjectsTransformationCommand::Map map;
  for (auto&& object : objects) {
    map.insert(std::pair(object, new_transformations.at(object)));
  }
  return map;
}

}  // namespace

namespace omm
{
ObjectsTransformationCommand::ObjectsTransformationCommand(const Map& transformations,
                                                           TransformationMode t_mode)
    : Command(QObject::tr("ObjectsTransformation")),
      m_old_transformations(get_old_transformations(::get_keys(transformations))),
      m_new_transformations(get_new_transformations(transformations)), m_transformation_mode(t_mode)
{
}

void ObjectsTransformationCommand::undo()
{
  apply(m_old_transformations);
}
void ObjectsTransformationCommand::redo()
{
  apply(m_new_transformations);
}

bool ObjectsTransformationCommand::is_noop() const
{
  assert(::get_keys(m_old_transformations) == ::get_keys(m_new_transformations));
  return std::all_of(m_old_transformations.begin(), m_old_transformations.end(), [this](auto&& oo) {
    const auto [i, _] = oo;
    return m_old_transformations.at(i) == m_new_transformations.at(i);
  });
}

bool ObjectsTransformationCommand::mergeWith(const QUndoCommand* command)
{
  const auto& ot_command = dynamic_cast<const ObjectsTransformationCommand&>(*command);
  if (affected_objects() != ot_command.affected_objects()) {
    return false;
  }

  for (auto&& object : affected_objects()) {
    m_new_transformations[object] = ot_command.m_new_transformations.at(object);
  }
  return true;
}

void ObjectsTransformationCommand::apply(const ObjectsTransformationCommand::Map& map)
{
  for (auto&& [o, t] : map) {
    switch (m_transformation_mode) {
    case TransformationMode::Axis:
      o->set_global_axis_transformation(t, Space::Scene);
      o->update();
      break;
    case TransformationMode::Object:
      o->set_global_transformation(t, Space::Scene);
      break;
    }
  }
}

std::set<Object*> ObjectsTransformationCommand::affected_objects() const
{
  auto keys = ::get_keys(m_new_transformations);
  assert(keys == ::get_keys(m_old_transformations));
  return keys;
}

int ObjectsTransformationCommand::id() const
{
  return OBJECTS_TRANSFORMATION_COMMAND_ID;
}

}  // namespace omm
