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
  for (auto&& handle : handles) {
    handle->draw(renderer);
  }
}

void PathTool::set_selection(const std::set<Object*>& objects)
{
  const auto paths = ::filter_if(objects, [](const Object* object) {
    return object->type() == Path::TYPE;
  });
  m_paths = ::transform<Path*>(paths, [](Object* object) { return static_cast<Path*>(object); });

  Tool::set_selection(paths);

  handles.clear();
  handles.reserve(m_paths.size());
  std::transform(m_paths.begin(), m_paths.end(), std::back_inserter(handles), [](Path* path) {
    return std::make_unique<PathHandle>(*path);
  });
}

}  // namespace omm
