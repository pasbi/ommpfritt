#pragma once

#include "objects/object.h"
#include "tools/positionvariant.h"
#include "tools/handles/selecthandle.h"
#include "tools/tool.h"
#include <Qt>

namespace omm
{

class Command;

class AbstractSelectTool : public Tool
{
public:
  explicit AbstractSelectTool(Scene& scene);
  static constexpr auto ALIGNMENT_PROPERTY_KEY = "alignment";
  virtual Command* transform_objects(ObjectTransformation t, const bool tool_space) = 0;
  Command *transform_objects_absolute(ObjectTransformation t, const bool tool_space);
  void cancel() override;
  bool mouse_move(const arma::vec2 &delta, const arma::vec2 &pos, const QMouseEvent &e);
  bool mouse_press(const arma::vec2 &pos, const QMouseEvent &e, bool force);
  void draw(AbstractRenderer &renderer) const;
  std::string tool_info;
  void mouse_release(const arma::vec2 &pos, const QMouseEvent &event);

private:
  ObjectTransformation m_last_object_transformation;
  void reset_absolute_object_transformation();
  const Style m_tool_info_line_style;
  arma::vec2 m_init_position;
  arma::vec2 m_current_position;
};

template<typename PositionVariant>
class SelectTool : public AbstractSelectTool
{
public:
  explicit SelectTool(Scene& scene);
  bool mouse_press(const arma::vec2& pos, const QMouseEvent& event, bool force) override;
  void on_scene_changed() override;
  PositionVariant position_variant;
  ObjectTransformation transformation() const override;
  bool has_transformation() const override;

};

class SelectObjectsTool : public SelectTool<ObjectPositions>
{
public:
  explicit SelectObjectsTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SelectObjectsTool");
  std::string name() const override;
  Command* transform_objects(ObjectTransformation t, const bool tool_space) override;
  static constexpr auto TRANSFORMATION_MODE_KEY = "transformation_mode";

};

class SelectPointsTool : public SelectTool<PointPositions>
{
public:
  explicit SelectPointsTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SelectPointsTool");
  static constexpr auto TANGENT_MODE_PROPERTY_KEY = "tangent_mode";
  PointSelectHandle::TangentMode tangent_mode() const;
  std::unique_ptr<QMenu> make_context_menu(QWidget* parent) override;
  void on_selection_changed() override;
  std::string name() const override;
  Command *transform_objects(ObjectTransformation t, const bool tool_space) override;

};

}  // namespace omm
