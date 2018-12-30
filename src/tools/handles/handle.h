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
  explicit Handle();
  virtual ~Handle() = default;
  virtual void draw(AbstractRenderer& renderer) const = 0;
  void mouse_press(const arma::vec2& pos);
  virtual void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover);
  void mouse_release();
  Status status() const;
  void deactivate();
  void set_style(Status status, Style&& style);

protected:
  virtual bool contains(const arma::vec2& point) const = 0;
  const Style& current_style() const;

private:
  Status m_status = Status::Inactive;
  std::map<Status, Style> m_styles;
};

}  // namespace omm
