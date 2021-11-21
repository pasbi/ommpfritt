#include "tools/transformpointstool.h"
#include "tools/handles/boundingboxhandle.h"
#include "tools/handles/moveaxishandle.h"
#include "tools/handles/rotatehandle.h"
#include "tools/handles/scaleaxishandle.h"
#include "tools/handles/scalebandhandle.h"
#include "scene/scene.h"

namespace omm
{

QString TransformPointsTool::type() const
{
  return TYPE;
}

void TransformPointsTool::reset()
{
  SelectPointsBaseTool::reset();
  push_handle(std::make_unique<ScaleBandHandle<TransformPointsTool>>(*this));
  push_handle(std::make_unique<RotateHandle<TransformPointsTool>>(*this));

  static constexpr auto X = AxisHandleDirection::X;
  static constexpr auto Y = AxisHandleDirection::Y;
  push_handle(std::make_unique<MoveAxisHandle<TransformPointsTool, X>>(*this));
  push_handle(std::make_unique<MoveAxisHandle<TransformPointsTool, Y>>(*this));
  push_handle(std::make_unique<ScaleAxisHandle<TransformPointsTool, X>>(*this));
  push_handle(std::make_unique<ScaleAxisHandle<TransformPointsTool, Y>>(*this));
  push_handle(std::make_unique<MoveParticleHandle<TransformPointsTool>>(*this));
  push_handle(std::make_unique<BoundingBoxHandle<TransformPointsTool>>(*this));
}

}  // namespace omm
