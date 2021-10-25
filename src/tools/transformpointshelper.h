#pragma once

#include <QObject>
#include <set>
#include "commands/pointstransformationcommand.h"

namespace omm
{

class Scene;
class ObjectTransformation;
class Path;

class TransformPointsHelper : public QObject
{
  Q_OBJECT
public:
  explicit TransformPointsHelper(Scene& scene, Space space);
  [[nodiscard]] std::unique_ptr<PointsTransformationCommand> make_command(const ObjectTransformation& t) const;
  void update(const std::set<Path*>& paths);
  void update();
  [[nodiscard]] bool is_empty() const { return m_initial_points.empty(); }

Q_SIGNALS:
  void initial_transformations_changed();

private:
  PointsTransformationCommand::Map m_initial_points;
  std::set<Path*> m_paths;
  Scene& m_scene;
  const Space m_space;
};

}  // namespace omm
