#pragma once

#include "commands/objectstransformationcommand.h"
#include <QObject>
#include <set>


namespace omm
{

class TransformObjectsHelper : public QObject
{
  Q_OBJECT
public:
  explicit TransformObjectsHelper();
  using TransformationMode = ObjectsTransformationCommand::TransformationMode;

  /**
   * @brief make_command makes the command to transform the selected objects
   * @param t the transformation to apply to each object
   * @param mode
   * @return
   */
  [[nodiscard]] std::unique_ptr<ObjectsTransformationCommand>
  make_command(const Matrix& t, TransformationMode mode = TransformationMode::Object) const;

  void update(const std::set<Object*>& objects);
  void update();
  [[nodiscard]] bool is_empty() const
  {
    return m_initial_transformations.empty();
  }

Q_SIGNALS:
  void initial_transformations_changed();

private:
  ObjectsTransformationCommand::Map m_initial_transformations;
  std::set<Object*> m_objects;
};

}  // namespace omm
