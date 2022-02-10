#include "tools/handles/objectselecthandle.h"
#include "geometry/vec2.h"
#include "geometry/objecttransformation.h"
#include "scene/scene.h"
#include "tools/tool.h"
#include <QPainter>

namespace omm
{

ObjectSelectHandle::ObjectSelectHandle(Tool& tool, Scene& scene, Object& object)
    : AbstractSelectHandle(tool), m_scene(scene), m_object(object)
{
}

bool ObjectSelectHandle::contains_global(const Vec2f& point) const
{
  return (point - transformation().null()).max_norm() < interact_epsilon();
}

void ObjectSelectHandle::draw(QPainter& painter) const
{
  const auto is_selected = m_scene.item_selection<Object>().contains(&m_object);

  const auto pos = transformation().null();
  const auto r = draw_epsilon();

  const auto status = is_selected ? HandleStatus::Active : this->status();
  painter.setPen(ui_color(status, "object"));
  painter.setBrush(ui_color(status, "object fill"));
  painter.drawRect(Tool::centered_rectangle(pos, r));
}

ObjectTransformation ObjectSelectHandle::transformation() const
{
  return m_object.global_transformation(Space::Viewport);
}

void ObjectSelectHandle::clear()
{
  m_scene.set_selection({});
}

void ObjectSelectHandle::set_selected(bool selected)
{
  auto selection = m_scene.item_selection<Object>();
  if (selected) {
    selection.insert(&m_object);
  } else {
    selection.erase(&m_object);
  }
  m_scene.set_selection(down_cast(selection));
}

bool ObjectSelectHandle::is_selected() const
{
  return m_scene.item_selection<Object>().contains(&m_object);
}

}  // namespace omm
