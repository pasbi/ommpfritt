#pragma once

#include <armadillo>
#include "tools/handles/handle.h"
#include "geometry/util.h"

namespace omm
{

class BandHandle : public Handle
{
public:
  explicit BandHandle(Tool& tool, bool transform_in_tool_space);
  bool contains_global(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;

private:
  static constexpr double width = 40.0;
  static constexpr double r = 70.0;
  static constexpr double stop = 10.0;
};

template<typename ToolT>
class ScaleBandHandle : public BandHandle
{
public:
  ScaleBandHandle(ToolT& tool) : BandHandle(tool, true), m_tool(tool) {}
  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e) override
  {
    BandHandle::mouse_move(delta, pos, e);
    if (status() == Status::Active) {
      const auto ti = m_tool.transformation().inverted();
      arma::vec2 gpos = ti.apply_to_position(pos);
      arma::vec2 gdelta = ti.apply_to_direction(delta);
      const auto t = omm::ObjectTransformation().scaled(gpos / (gpos - gdelta));
      m_tool.transform_objects(t, true);
      return true;
    } else {
      return false;
    }
  }

private:
  ToolT& m_tool;
};

}  // namespace omm
