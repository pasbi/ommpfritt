#pragma once

#include "tools/selectpointstool.h"

namespace omm
{

class KnifeTool : public SelectPointsBaseTool
{
public:
  explicit KnifeTool(Scene& scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "KnifeTool");

  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent&) override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event) override;
  void mouse_release(const Vec2f& pos, const QMouseEvent& event) override;
  QString type() const override;
  void draw(Painter& renderer) const override;
  QString name() const override;
  bool cancel() override;
  SceneMode scene_mode() const override;

private:
  bool m_is_cutting = false;
  Vec2f m_mouse_press_pos;
  Vec2f m_mouse_move_pos;
  std::list<Point> m_points;
};

}  // namespace omm
