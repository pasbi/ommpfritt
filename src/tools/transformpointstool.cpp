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
  make_handles(*this, false);
  handles.push_back(std::make_unique<ScaleBandHandle<TransformPointsTool>>(*this));
  handles.push_back(std::make_unique<RotateHandle<TransformPointsTool>>(*this));

  static constexpr auto X = AxisHandleDirection::X;
  static constexpr auto Y = AxisHandleDirection::Y;
  handles.push_back(std::make_unique<MoveAxisHandle<TransformPointsTool, X>>(*this));
  handles.push_back(std::make_unique<MoveAxisHandle<TransformPointsTool, Y>>(*this));
  handles.push_back(std::make_unique<ScaleAxisHandle<TransformPointsTool, X>>(*this));
  handles.push_back(std::make_unique<ScaleAxisHandle<TransformPointsTool, Y>>(*this));
  handles.push_back(std::make_unique<MoveParticleHandle<TransformPointsTool>>(*this));
  handles.push_back(std::make_unique<BoundingBoxHandle<TransformPointsTool>>(*this));
}

}  // namespace omm
