#pragma once

#include <armadillo>
#include "tools/handles/handle.h"
#include "geometry/util.h"

namespace omm
{

class ParticleHandle : public Handle
{
public:
  explicit ParticleHandle();
  bool contains(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;
};

template<typename ToolT>
class PointMoveHandle : public omm::ParticleHandle
{
public:
  PointMoveHandle(ToolT& tool) : m_tool(tool) {}
  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override
  {
    ParticleHandle::mouse_move(delta, pos, allow_hover);
    if (status() == Status::Active) {
      const auto t = omm::ObjectTransformation().translated(delta);
      m_tool.transform_objects(t);
    }
  }

private:
  ToolT& m_tool;
};

template<typename ToolT>
class PointScaleHandle : public omm::ParticleHandle
{
public:
  PointScaleHandle(ToolT& tool) : m_tool(tool) {}
  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override
  {
    ParticleHandle::mouse_move(delta, pos, allow_hover);
    if (status() == Status::Active) {
      const auto scale = get_scale(pos, delta, arma::vec2{ 1.0, 1.0 });
      const auto t = omm::ObjectTransformation().scaled(scale);
      m_tool.transform_objects(t);
    }
  }

private:
  ToolT& m_tool;
};

}  // namespace omm
