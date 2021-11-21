#pragma once

#include "tools/selectpointstool.h"
#include <QCoreApplication>

namespace omm
{
class BrushSelectTool : public SelectPointsBaseTool
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
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event) override;
  void mouse_release(const Vec2f& pos, const QMouseEvent& event) override;
  QString type() const override;
  void draw(Painter& renderer) const override;
  static constexpr auto RADIUS_PROPERTY_KEY = "radius";
  QString name() const override;
  SceneMode scene_mode() const override
  {
    return SceneMode::Vertex;
  }

private:
  bool m_mouse_down = false;
  Vec2f m_mouse_pos;
  void modify_selection(const Vec2f& pos, const QMouseEvent& event);
};

}  // namespace omm
