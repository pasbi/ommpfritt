#pragma once

#include <Qt>
#include "renderers/style.h"
#include "geometry/objecttransformation.h"

class QMouseEvent;

namespace omm
{

class Painter;
class Tool;

class Handle
{
public:
  enum class Status { Hovered, Active, Inactive };
  explicit Handle(Tool& tool, const bool transform_in_tool_space);
  virtual ~Handle() = default;
  virtual void draw(Painter& renderer) const = 0;
  virtual bool mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force);
  virtual bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent&);
  virtual void mouse_release(const Vec2f& pos, const QMouseEvent&);
  Status status() const;
  virtual void deactivate();
  virtual double draw_epsilon() const;
  virtual double interact_epsilon() const;
  const bool transform_in_tool_space;
  bool is_enabled() const;

protected:
  Vec2f transform_position_to_global(const Vec2f& position) const;
  virtual bool contains_global(const Vec2f& global_point) const = 0;
  void set_style(Status status, Style style);
  const Style& current_style() const;
  const Style& style(Status status) const;
  virtual ObjectTransformation transformation() const;
  Tool& tool;
  Vec2f press_pos() const;

  void discretize(Vec2f& vec) const;

private:
  Status m_status = Status::Inactive;
  std::map<Status, Style> m_styles;
  bool m_enabled = false;
  Vec2f m_press_pos;
};

}  // namespace omm
