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
  virtual ~Handle() = default;
  virtual void draw(AbstractRenderer& renderer) const = 0;
  virtual bool mouse_press(const arma::vec2& pos, const QMouseEvent& event);
  virtual bool mouse_move( const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e );
  virtual void mouse_release(const arma::vec2& pos, const QMouseEvent& event);
  Status status() const;
  virtual void deactivate();
  void set_style(Status status, Style&& style);
  double epsilon = 4.0;

protected:
  virtual bool contains(const arma::vec2& point) const = 0;
  virtual bool contains_global(const arma::vec2& global_point) const;
  const Style& current_style() const;
  const Style& style(Status status) const;

  virtual ObjectTransformation transformation() const;

private:
  Status m_status = Status::Inactive;
  std::map<Status, Style> m_styles;
};

}  // namespace omm
