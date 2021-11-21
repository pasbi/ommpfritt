#pragma once

#include "tools/handles/handle.h"

namespace omm
{

class Scene;

class AbstractSelectHandle : public Handle
{
public:
  explicit AbstractSelectHandle(Tool& tool);
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event) override;
  void mouse_release(const Vec2f& pos, const QMouseEvent& event) override;
  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override;

protected:
  virtual void set_selected(bool selected) = 0;
  virtual void clear() = 0;
  [[nodiscard]] virtual bool is_selected() const = 0;
  static constexpr auto extend_selection_modifier = Qt::ShiftModifier;

private:
  bool m_was_selected = false;
};

}  // namespace omm
