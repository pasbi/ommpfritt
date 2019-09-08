#pragma once

#include "tools/selecttool.h"
#include "commands/objectstransformationcommand.h"

namespace omm
{

class TransformObjectsHelper
{
public:
  explicit TransformObjectsHelper();
  using TransformationMode = ObjectsTransformationCommand::TransformationMode;

  /**
   * @brief make_command makes the command to transform the selected objects
   * @param t the transformation to apply to each object
   * @param mode
   * @return
   */
  std::unique_ptr<ObjectsTransformationCommand>
  make_command(const Matrix &t,
               TransformationMode mode = TransformationMode::Object) const;

  void update(const std::set<Object*>& objects);
  void update();
  bool is_empty() const { return m_initial_transformations.size() == 0; }

private:
  ObjectsTransformationCommand::Map m_initial_transformations;
  std::set<Object*> m_objects;
};

class SelectObjectsTool : public AbstractSelectTool
{
public:
  explicit SelectObjectsTool(Scene& scene);
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SelectObjectsTool");

  /**
   * @brief transform_objects transforms the selected objects with given transformation @code t.
   * @param t the transformation in viewport-space.
   */
  void transform_objects(ObjectTransformation t) override;
  static constexpr auto TRANSFORMATION_MODE_KEY = "transformation_mode";
  void reset() override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force) override;
  BoundingBox bounding_box() const;

protected:
  bool has_transformation() const override;
  Vec2f selection_center() const override;
  TransformObjectsHelper m_transform_objects_helper;

};

}  // namespace omm
