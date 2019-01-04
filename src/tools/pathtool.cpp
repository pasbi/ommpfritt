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

void PathTool::draw(AbstractRenderer& renderer) const
{
  const auto style = SolidStyle(Color(1, 0, 1));
  for (auto&& path : paths()) {
    renderer.push_transformation(path->global_transformation());
    for (Point* p : path->points()) {
      renderer.draw_rectangle(p->position, 10, style);
    }
    renderer.pop_transformation();
  }
}

std::set<Path*> PathTool::paths() const
{
  const auto is_path = [](const Object* object) { return object->type() == Path::TYPE; };
  const auto to_path = [](Object* object) { return static_cast<Path*>(object); };
  return ::transform<Path*>(::filter_if(selection(), is_path), to_path);
}

void PathTool::set_selection(const std::set<Object*>& objects)
{
  Tool::set_selection(objects);
  
}

}  // namespace omm
