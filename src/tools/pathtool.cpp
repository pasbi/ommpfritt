#include "tools/pathtool.h"
#include <memory>
#include "scene/scene.h"
#include "objects/path.h"

namespace omm
{

std::string PathTool::type() const
{
  return TYPE;
}

QIcon PathTool::icon() const
{
  return QIcon();
}

// void PathTool::draw(AbstractRenderer& renderer)
// {
//   const auto style = SolidStyle(Color(1, 0, 1));
//   for (auto&& handle : handles) {
//     handle->draw(renderer);
//   }
// }

}  // namespace omm
