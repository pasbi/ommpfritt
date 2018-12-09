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
  enum class Status { Hovered, Active, Inactive };
  explicit SubHandle(Handle& handle);
  virtual ~SubHandle() = default;
  virtual void draw(AbstractRenderer& renderer) const = 0;
  void mouse_press(const arma::vec2& pos);
  virtual void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover);
  void mouse_release();
  Status status() const;
  void deactivate();

protected:
  virtual bool contains(const arma::vec2& point) const = 0;
  Handle& handle;

private:
  Status m_status = Status::Inactive;
};

}  // namespace omm
