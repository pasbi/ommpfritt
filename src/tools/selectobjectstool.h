#pragma once

#include "commands/objectstransformationcommand.h"
#include "tools/selecttool.h"

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

class SelectObjectsTool : public AbstractSelectTool
{
public:
  explicit SelectObjectsTool(Scene& scene);
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SelectObjectsTool");

  /**
   * @brief transform_objects transforms the selected objects with given transformation @code t.
   * @param t the transformation in viewport-space.
   */
  void transform_objects(ObjectTransformation t) override;
  static constexpr auto TRANSFORMATION_MODE_KEY = "transformation_mode";
  void reset() override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event) override;
  void mouse_release(const Vec2f& pos, const QMouseEvent& event) override;
  BoundingBox bounding_box() const;
  SceneMode scene_mode() const override
  {
    return SceneMode::Object;
  }

protected:
  bool has_transformation() const override;
  Vec2f selection_center() const override;
  TransformObjectsHelper m_transform_objects_helper;
};

}  // namespace omm
