#pragma once

#include "commands/modifypointscommand.h"
#include "common.h"
#include <QObject>
#include <set>
#include <list>

namespace omm
{

class Scene;
class ObjectTransformation;
class PathObject;

class TransformPointsHelper : public QObject
{
  Q_OBJECT
public:
  explicit TransformPointsHelper(Scene& scene, Space space);
  [[nodiscard]] std::unique_ptr<ModifyPointsCommand> make_command(const ObjectTransformation& t) const;
  void update(const std::set<PathObject*>& path_objects);
  void update();
  [[nodiscard]] bool is_empty() const { return m_initial_points.empty(); }

Q_SIGNALS:
  void initial_transformations_changed();

private:
  std::set<PathObject*> m_path_objects;
  std::map<PathPoint*, Point> m_initial_points;
  Scene& m_scene;
  const Space m_space;
};

}  // namespace omm
