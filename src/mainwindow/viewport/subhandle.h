#pragma once

#include "renderers/style.h"
#include "geometry/objecttransformation.h"

namespace omm
{

class AbstractRenderer;
class Handle;

class SubHandle
{
public:
  explicit SubHandle(Handle& handle);
  virtual void draw(AbstractRenderer& renderer) const = 0;
  bool mouse_press(const arma::vec2& pos);
  virtual bool mouse_move(const arma::vec2& delta, const arma::vec2& pos);
  void mouse_release();

protected:
  virtual bool contains(const arma::vec2& point) const = 0;
  bool is_hovered() const;
  bool is_active() const;
  Handle& handle;

private:
  bool m_is_hovered = false;
  bool m_is_active = false;
};

class AxisHandle : public SubHandle
{
public:
  enum class Axis { X, Y };
  AxisHandle(Handle& handle, Axis axis);
  void draw(AbstractRenderer& renderer) const override;
  bool contains(const arma::vec2& point) const override;
  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos) override;

private:
  arma::vec2 project(const arma::vec2& point) const;
  static constexpr double LENGTH = 100;
  const arma::vec2 m_tip_position;
  const Color m_base_color;
};

}  // namespace omm
