#pragma once

#include <Qt>
#include "renderers/style.h"
#include "geometry/objecttransformation.h"

class QMouseEvent;

namespace omm
{

class AbstractRenderer;
class Tool;

class Handle
{
public:
  enum class Status { Hovered, Active, Inactive };
  explicit Handle(Tool& tool, const bool transform_in_tool_space);
  virtual ~Handle() = default;
  virtual void draw(AbstractRenderer& renderer) const = 0;
  virtual bool mouse_press(const arma::vec2& pos, const QMouseEvent& event, bool force);
  virtual bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent&);
  virtual void mouse_release(const arma::vec2& pos, const QMouseEvent&);
  Status status() const;
  virtual void deactivate();
  virtual double draw_epsilon() const;
  virtual double interact_epsilon() const;
  const bool transform_in_tool_space;
  bool is_enabled() const;

protected:
  arma::vec2 transform_position_to_global(const arma::vec2& position) const;
  virtual bool contains_global(const arma::vec2& global_point) const = 0;
  void set_style(Status status, Style style);
  const Style& current_style() const;
  const Style& style(Status status) const;
  virtual ObjectTransformation transformation() const;
  Tool& tool;
  arma::vec2 press_pos() const;

  void discretize(arma::vec2& vec) const;

private:
  Status m_status = Status::Inactive;
  std::map<Status, Style> m_styles;
  bool m_enabled = false;
  arma::vec2 m_press_pos;
};

}  // namespace omm
