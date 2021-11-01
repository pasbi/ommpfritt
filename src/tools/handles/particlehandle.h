#pragma once

#include "geometry/vec2.h"
#include "tools/handles/handle.h"
#include "tools/tool.h"

namespace omm
{
class ParticleHandle : public Handle
{
public:
  explicit ParticleHandle(Tool& tool);
  [[nodiscard]] bool contains_global(const Vec2f& point) const override;
  void draw(QPainter& painter) const override;
  Vec2f position = Vec2f::o();

protected:
  bool transform_in_tool_space{};
};

template<typename ToolT> class MoveParticleHandle : public ParticleHandle
{
public:
  MoveParticleHandle(ToolT& tool) : ParticleHandle(tool)
  {
  }

  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() == HandleStatus::Active) {
      static constexpr double STEP = 10.0;
      const auto inv_tool_transformation = tool.transformation().inverted();
      auto total_delta = inv_tool_transformation.apply_to_direction(pos - press_pos());
      total_delta = discretize(total_delta, false, STEP);
      {
        auto transformation = omm::ObjectTransformation().translated(total_delta);
        transformation = transformation.transformed(inv_tool_transformation);
        static_cast<ToolT&>(tool).transform_objects(transformation);
      }
      total_delta = tool.viewport_transformation.inverted().apply_to_direction(total_delta);
      const auto tool_info = QString("%1").arg(total_delta.euclidean_norm());
      static_cast<ToolT&>(tool).tool_info = tool_info;
      return true;
    } else {
      return false;
    }
  }
};

}  // namespace omm
