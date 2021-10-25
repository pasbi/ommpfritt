#include "tools/transformobjectshelper.h"
#include "objects/object.h"

namespace omm
{

TransformObjectsHelper::TransformObjectsHelper()
{
  update();
}

std::unique_ptr<ObjectsTransformationCommand>
TransformObjectsHelper::make_command(const Matrix& t, TransformationMode mode) const
{
  ObjectsTransformationCommand::Map map;
  for (auto&& [object, transformation] : m_initial_transformations) {
    map.insert(std::pair(object, ObjectTransformation(t * transformation.to_mat())));
  }
  return std::make_unique<ObjectsTransformationCommand>(map, mode);
}

void TransformObjectsHelper::update(const std::set<Object*>& objects)
{
  m_objects = objects;
  update();
}

void TransformObjectsHelper::update()
{
  m_initial_transformations.clear();
  for (auto&& o : m_objects) {
    m_initial_transformations.insert(std::pair(o, o->global_transformation(Space::Scene)));
  }
  Q_EMIT initial_transformations_changed();
}

}  // namespace omm
