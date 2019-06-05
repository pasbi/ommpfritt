#pragma once

#include "tools/tool.h"
#include "tools/selectpointstool.h"
#include <QCoreApplication>

namespace omm
{

class BrushSelectTool : public SelectPointsTool
{
public:
  explicit BrushSelectTool(Scene& scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "BrushSelectTool");

  /**
   * @brief returns true if any grabbable was moved
   */
  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& event) override;

  /**
   * @brief returns true if any grabbable was hit
   */
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force) override;
  void mouse_release(const Vec2f& pos, const QMouseEvent& event) override;
  std::string type() const override;
  void on_scene_changed() override;
  void on_selection_changed() override;
  void draw(Painter& renderer) const override;
  static constexpr auto RADIUS_PROPERTY_KEY = "radius";
  std::string name() const override;

private:
  bool m_mouse_down = false;
  Vec2f m_mouse_pos;
  const static Style m_style;
  void modify_selection(const Vec2f& pos, const QMouseEvent& event);
};

void register_tools();

}  // namespace omm
