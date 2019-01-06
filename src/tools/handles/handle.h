#pragma once

#include "renderers/style.h"
#include "geometry/objecttransformation.h"

namespace omm
{

class AbstractRenderer;
class Tool;

class Handle
{
public:
  enum class Status { Hovered, Active, Inactive };
  virtual ~Handle() = default;
  virtual void draw(AbstractRenderer& renderer) const = 0;
  virtual bool mouse_press(const arma::vec2& pos);
  virtual bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover);
  virtual void mouse_release();
  Status status() const;
  virtual void deactivate();
  void set_style(Status status, Style&& style);
  double epsilon = 10.0;

protected:
  virtual bool contains(const arma::vec2& point) const = 0;
  const Style& current_style() const;
  const Style& style(Status status) const;

  virtual ObjectTransformation transformation() const;

private:
  Status m_status = Status::Inactive;
  std::map<Status, Style> m_styles;
};

}  // namespace omm
