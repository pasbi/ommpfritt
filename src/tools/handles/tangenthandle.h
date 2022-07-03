#pragma once

#include "tools/handles/handle.h"
#include "geometry/point.h"

namespace omm
{

class PointSelectHandle;

class TangentHandle : public Handle
{
public:
  TangentHandle(Tool& tool, PointSelectHandle& master_handle, const Point::TangentKey& tangent_key);
  [[nodiscard]] double draw_epsilon() const override;
  void draw(QPainter& painter) const override;
  [[nodiscard]] bool contains_global(const Vec2f& point) const override;
  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override;

public:
  Vec2f position = Vec2f::o();

private:
  PointSelectHandle& m_master_handle;
  const Point::TangentKey m_tangent_key;
};

}  // namespace omm
