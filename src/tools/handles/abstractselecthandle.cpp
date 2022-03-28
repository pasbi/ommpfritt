#include "tools/handles/abstractselecthandle.h"
#include "scene/scene.h"
#include "tools/tool.h"
#include "scene/mailbox.h"
#include "tools/selecttool.h"
#include <QMouseEvent>
#include <QSignalBlocker>

namespace omm
{
AbstractSelectHandle::AbstractSelectHandle(Tool& tool)
  : Handle(tool)
{
}

bool AbstractSelectHandle::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  m_was_selected = is_selected();
  if (Handle::mouse_press(pos, event)) {
    if (!is_selected() && event.modifiers() != extend_selection_modifier) {
      clear();
    }
    set_selected(true);
    return true;
  } else {
    return false;
  }
}

void AbstractSelectHandle::mouse_release(const Vec2f& pos, const QMouseEvent& event)
{
  if (status() == HandleStatus::Active) {
    static constexpr double eps = 0.0;
    if ((press_pos() - pos).euclidean_norm2() <= eps) {
      if (m_was_selected) {
        if (event.modifiers() == extend_selection_modifier) {
          set_selected(false);
        } else {
          clear();
          set_selected(true);
        }
      }
    }
  }
  Handle::mouse_release(pos, event);
}

bool AbstractSelectHandle::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e)
{
  static constexpr double STEP_SIZE = 10.0;
  Handle::mouse_move(delta, pos, e);
  if (status() == HandleStatus::Active) {
    Vec2f total_delta = discretize(pos - press_pos(), false, STEP_SIZE);
    const auto transformation = omm::ObjectTransformation().translated(total_delta);
    total_delta = tool.viewport_transformation.inverted().apply_to_direction(total_delta);
    dynamic_cast<AbstractSelectTool&>(tool).transform_objects(transformation);
    const auto tool_info = QString("%1").arg(total_delta.euclidean_norm());
    dynamic_cast<AbstractSelectTool&>(tool).tool_info = tool_info;
    return true;
  } else {
    return false;
  }
}

}  // namespace omm
