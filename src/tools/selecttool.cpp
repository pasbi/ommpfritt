#include "tools/selecttool.h"
#include <memory>
#include <algorithm>
#include "scene/scene.h"
#include "objects/path.h"
#include "menuhelper.h"
#include "properties/floatproperty.h"
#include "properties/optionsproperty.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include <QCoreApplication>
#include "commands/objectstransformationcommand.h"
#include "commands/pointstransformationcommand.h"
#include "logging.h"

namespace omm
{

// TODO 1) implement a kd-tree or similar to get the closest object fast
//      2) I guess it's more intuitive when items can be selected by clicking anywhere inside
//          their area.
//      3) improve mouse pointer icon


AbstractSelectTool::AbstractSelectTool(Scene& scene)
  : Tool(scene)
  , m_tool_info_line_style(ContourStyle(Color(0.0, 0.0, 0.0, 0.3), 0.7))

{
  this->add_property<OptionsProperty>(ALIGNMENT_PROPERTY_KEY, 1)
    .set_options({ QObject::tr("global").toStdString(), QObject::tr("local").toStdString() })
    .set_label(QObject::tr("Alignment").toStdString())
    .set_category(QObject::tr("tool").toStdString());
}

Command*
AbstractSelectTool::transform_objects_absolute(ObjectTransformation t, const bool tool_space)
{
  Command* cmd = transform_objects(m_last_object_transformation.inverted().apply(t), tool_space);
  m_last_object_transformation = t;
  return cmd;
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


bool AbstractSelectTool::mouse_press(const Vec2f& pos, const QMouseEvent& e, bool force)
{
  const bool r = Tool::mouse_press(pos, e, force);
  reset_absolute_object_transformation();
  return r;
}

void AbstractSelectTool::mouse_release(const Vec2f &pos, const QMouseEvent &event)
{
  tool_info.clear();
  Tool::mouse_release(pos, event);
}

ObjectTransformation AbstractSelectTool::transformation() const
{
  ObjectTransformation transformation;
  transformation.translate(selection_center());
  if (this->property(ALIGNMENT_PROPERTY_KEY)->template value<size_t>() == 1) {
    if (scene.item_selection<Object>().size() == 1) {
      const auto* lonely_object = *scene.item_selection<Object>().begin();
      transformation.rotate(lonely_object->global_transformation().rotation());
    }
  }
  return transformation;
}

void AbstractSelectTool::draw(Painter &renderer) const
{
  Tool::draw(renderer);
  if (!tool_info.empty()) {
    renderer.toast(m_current_position + Vec2f(30.0, 30.0), tool_info.c_str());
    const auto line = std::vector { Point(m_init_position), Point(m_current_position) };

    renderer.set_style(m_tool_info_line_style);
    renderer.painter->drawLine(m_init_position.x, m_init_position.y,
                               m_current_position.x, m_current_position.y);
  }
}

void AbstractSelectTool::cancel()
{
  Command* cmd = transform_objects_absolute(ObjectTransformation(), true);
  if (cmd) {
    cmd->setObsolete(true);
    scene.history.undo();
  }
  Tool::cancel();
}

}  // namespace omm
