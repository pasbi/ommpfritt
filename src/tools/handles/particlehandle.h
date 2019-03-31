#pragma once

#include <armadillo>
#include "tools/handles/handle.h"
#include "geometry/util.h"
#include "tools/tool.h"

namespace omm
{

class ParticleHandle : public Handle
{
public:
  explicit ParticleHandle(Tool& tool, bool transform_in_tool_space);
  bool contains_global(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;
  arma::vec2 position = arma::vec2{ 0.0, 0.0 };
};

template<typename ToolT>
class MoveParticleHandle : public ParticleHandle
{
public:
  MoveParticleHandle(ToolT& tool) : ParticleHandle(tool, true) {}
  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() == Status::Active) {
      auto total_delta = transformation().inverted().apply_to_direction(pos - press_pos());
      discretize(total_delta);
      const auto transformation = omm::ObjectTransformation().translated(total_delta);
      static_cast<ToolT&>(tool).transform_objects_absolute(transformation, true);
      total_delta = tool.viewport_transformation.inverted().apply_to_direction(total_delta);
      const auto tool_info = QString("%1").arg(arma::norm(total_delta));
      static_cast<ToolT&>(tool).tool_info = tool_info.toStdString();
      return true;
    } else {
      return false;
    }
  }
};

}  // namespace omm
