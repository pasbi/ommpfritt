#pragma once

#include "common.h"
#include "geometry/vec2.h"
#include <QPalette>
#include <Qt>

class QMouseEvent;

namespace omm
{
class Painter;
class Tool;

enum class AxisHandleDirection { X, Y };

class Handle
{
public:
  explicit Handle(Tool& tool);
  virtual ~Handle() = default;
  Handle(Handle&&) = delete;
  Handle(const Handle&) = delete;
  Handle& operator=(Handle&&) = delete;
  Handle& operator=(const Handle&) = delete;

  virtual void draw(QPainter& renderer) const = 0;
  virtual bool mouse_press(const Vec2f& pos, const QMouseEvent& event);
  virtual bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent&);
  virtual void mouse_release(const Vec2f& pos, const QMouseEvent&);
  [[nodiscard]] HandleStatus status() const;
  virtual void deactivate();
  [[nodiscard]] virtual double draw_epsilon() const;
  [[nodiscard]] virtual double interact_epsilon() const;
  [[nodiscard]] virtual bool contains_global(const Vec2f& global_point) const = 0;

protected:
  Tool& tool;
  [[nodiscard]] Vec2f press_pos() const;

  [[nodiscard]] Vec2f discretize(const Vec2f& vec, bool local, double step) const;
  [[nodiscard]] static double discretize(double s, double step);

  [[nodiscard]] static QColor ui_color(HandleStatus status, const QString& name);
  [[nodiscard]] QColor ui_color(const QString& name) const;

private:
  HandleStatus m_status = HandleStatus::Inactive;
  Vec2f m_press_pos;

public:
  static const std::map<AxisHandleDirection, Vec2f> axis_directions;
  static const std::map<AxisHandleDirection, QString> axis_names;
};

}  // namespace omm
