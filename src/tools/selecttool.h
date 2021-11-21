#pragma once

#include "tools/handles/abstractselecthandle.h"
#include "tools/tool.h"
#include <Qt>

namespace omm
{
class Command;

class AbstractSelectTool : public Tool
{
public:
  explicit AbstractSelectTool(Scene& scene);
  enum class Alignment { Global, Local };
  static constexpr auto ALIGNMENT_PROPERTY_KEY = "alignment";
  static constexpr auto SYMMETRIC_PROPERTY_KEY = "symmetric";
  virtual void transform_objects(ObjectTransformation t) = 0;
  bool cancel() override;
  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& e) override;
  void draw(Painter& renderer) const override;
  QString tool_info;
  void mouse_release(const Vec2f& pos, const QMouseEvent& event) override;
  ObjectTransformation transformation() const override;

protected:
  void on_property_value_changed(Property* property) override;
  virtual Vec2f selection_center() const = 0;
  ObjectTransformation m_last_object_transformation;

private:
  void reset_absolute_object_transformation();
  Vec2f m_init_position;
  Vec2f m_current_position;
};

}  // namespace omm
