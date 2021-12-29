#include "tools/selecttool.h"
#include "commands/objectstransformationcommand.h"
#include "logging.h"
#include "main/application.h"
#include "mainwindow/mainwindow.h"
#include "menuhelper.h"
#include "objects/pathobject.h"
#include "preferences/uicolors.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"
#include "renderers/painter.h"
#include "renderers/painteroptions.h"
#include "scene/history/historymodel.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include <QCoreApplication>
#include <algorithm>
#include <memory>

namespace omm
{
// TODO 1) implement a kd-tree or similar to get the closest object fast
//      2) I guess it's more intuitive when items can be selected by clicking anywhere inside
//          their area.
//      3) improve mouse pointer icon

AbstractSelectTool::AbstractSelectTool(Scene& scene) : Tool(scene)

{
  const QString category = QObject::tr("tool");
  create_property<OptionProperty>(ALIGNMENT_PROPERTY_KEY, 1)
      .set_options({QObject::tr("global"), QObject::tr("local")})
      .set_label(QObject::tr("Alignment"))
      .set_category(category)
      .set_animatable(false);

  create_property<BoolProperty>(SYMMETRIC_PROPERTY_KEY, false)
      .set_label(QObject::tr("Symmetric"))
      .set_category(category)
      .set_animatable(false);
}

void AbstractSelectTool::reset_absolute_object_transformation()
{
  m_init_position = transformation().null();
  m_last_object_transformation = ObjectTransformation();
}

bool AbstractSelectTool::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e)
{
  m_current_position = transformation().null();
  return Tool::mouse_move(delta, pos, e);
}

bool AbstractSelectTool::mouse_press(const Vec2f& pos, const QMouseEvent& e)
{
  const bool r = Tool::mouse_press(pos, e);
  reset_absolute_object_transformation();
  return r;
}

void AbstractSelectTool::mouse_release(const Vec2f& pos, const QMouseEvent& event)
{
  tool_info.clear();
  Tool::mouse_release(pos, event);
}

ObjectTransformation AbstractSelectTool::transformation() const
{
  ObjectTransformation transformation;
  transformation.translate(selection_center());
  if (property(ALIGNMENT_PROPERTY_KEY)->template value<Alignment>() == Alignment::Local) {
    if (scene()->item_selection<Object>().size() == 1) {
      const auto* lonely_object = *scene()->item_selection<Object>().begin();
      transformation.rotate(lonely_object->global_transformation(Space::Viewport).rotation());
    }
  }
  return transformation;
}

void AbstractSelectTool::on_property_value_changed(Property* property)
{
  if (pmatch(property, {ALIGNMENT_PROPERTY_KEY})) {
    Q_EMIT scene()->mail_box().tool_appearance_changed(*this);
  } else {
    Tool::on_property_value_changed(property);
  }
}

void AbstractSelectTool::draw(Painter& renderer) const
{
  Tool::draw(renderer);
  if (!tool_info.isEmpty()) {
    static constexpr Vec2f TOAST_OFFSET{30.0, 30.0};
    // compute current position again, m_current_position is not yet updated.
    const auto current_position = transformation().null();
    renderer.toast(current_position + TOAST_OFFSET, tool_info);
    renderer.painter->setPen(ui_color(HandleStatus::Active, "Handle", "line"));
    renderer.painter->drawLine(static_cast<int>(m_init_position.x),
                               static_cast<int>(m_init_position.y),
                               static_cast<int>(current_position.x),
                               static_cast<int>(current_position.y));
  }
}

bool AbstractSelectTool::cancel()
{
  if (is_active()) {
    transform_objects(ObjectTransformation{});
    Tool::cancel();
    if (auto&& h = scene()->history(); h.last_command_is_noop()) {
      h.make_last_command_obsolete();
      scene()->history().undo();
    }
  } else {
    Tool::cancel();
  }

  return false;
}

}  // namespace omm
