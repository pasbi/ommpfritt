#include "tools/selecttool.h"
#include <memory>
#include <algorithm>
#include "tools/handles/axishandle.h"
#include "tools/handles/particlehandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"
#include "objects/path.h"

namespace omm
{

// TODO 1) implement a kd-tree or similar to get the closest object fast
//      2) I guess it's more intuitive when items can be selected by clicking anywhere inside
//          its area.
//      3) improve mouse pointer icon
//      4) select during mouse move (like drawing)

template<typename PositionVariant>
bool SelectTool<PositionVariant>::mouse_press(const arma::vec2& pos)
{
  if (!Tool::mouse_press(pos)) {
    m_position_variant.clear_selection();
  }
  return true;
}

template<typename PositionVariant>
void SelectTool<PositionVariant>::activate()
{
  handles.clear();
  m_position_variant.make_handles(handles);
}

std::string SelectObjectsTool::type() const
{
  return TYPE;
}

QIcon SelectObjectsTool::icon() const
{
  return QIcon();
}

std::string SelectPointsTool::type() const
{
  return TYPE;
}

QIcon SelectPointsTool::icon() const
{
  return QIcon();
}

template class SelectTool<ObjectPositions>;
template class SelectTool<PointPositions>;

}  // namespace omm
