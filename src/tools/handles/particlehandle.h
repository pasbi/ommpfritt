#pragma once

#include <armadillo>
#include "tools/handles/handle.h"
#include "geometry/util.h"

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
  MoveParticleHandle(ToolT& tool) : ParticleHandle(tool, true), m_tool(tool) {}
  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e) override
  {
    ParticleHandle::mouse_move(delta, pos, e);
    if (status() == Status::Active) {
      const auto t = omm::ObjectTransformation().translated(delta);
      m_tool.transform_objects(t, false);
      return true;
    } else {
      return false;
    }
  }

private:
  ToolT& m_tool;
};

}  // namespace omm
