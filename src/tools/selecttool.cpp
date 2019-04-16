#include "tools/selecttool.h"
#include <memory>
#include <algorithm>
#include "tools/handles/moveaxishandle.h"
#include "tools/handles/rotatehandle.h"
#include "tools/handles/scalebandhandle.h"
#include "tools/handles/particlehandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"
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
  this->add_property<OptionsProperty>(ALIGNMENT_PROPERTY_KEY)
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

void AbstractSelectTool::draw(AbstractRenderer& renderer) const
{
  Tool::draw(renderer);
  if (!tool_info.empty()) {
    renderer.toast(m_current_position + Vec2f(30.0, 30.0), tool_info.c_str());
    const auto line = std::vector { Point(m_init_position), Point(m_current_position) };
    renderer.draw_spline(line, m_tool_info_line_style, false);
  }
}

void AbstractSelectTool::cancel()
{
  Command* cmd = transform_objects_absolute(ObjectTransformation(), true);
  if (cmd) {
    cmd->setObsolete(true);
    scene.undo_stack.undo();
  }
  Tool::cancel();
}

template<typename PositionVariant>
SelectTool<PositionVariant>::SelectTool(Scene& scene)
  : AbstractSelectTool(scene)
  , position_variant(scene)
{}

template<typename PositionVariant>
ObjectTransformation SelectTool<PositionVariant>::transformation() const
{
  ObjectTransformation transformation;
  transformation.translate(position_variant.selection_center());
  if (this->property(ALIGNMENT_PROPERTY_KEY).template value<size_t>() == 1) {
    if (scene.item_selection<Object>().size() == 1) {
      const auto* lonely_object = *scene.item_selection<Object>().begin();
      transformation.rotate(lonely_object->global_transformation().rotation());
    }
  }
  return transformation;
}

template<typename PositionVariant> bool SelectTool<PositionVariant>
::mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force)
{
  Q_UNUSED(force);
  if (AbstractSelectTool::mouse_press(pos, event, false)) {
    return true;
  } else if (AbstractSelectTool::mouse_press(pos, event, true)) {
    return true;
  } else {
    position_variant.clear_selection();
    return false;
  }
}

template<typename PositionVariant>
void SelectTool<PositionVariant>::on_scene_changed()
{
  this->handles.clear();
  using tool_t = std::remove_pointer_t<decltype(this)>;
  handles.push_back(std::make_unique<ScaleBandHandle<tool_t>>(*this));
  handles.push_back(std::make_unique<RotateHandle<tool_t>>(*this));
  handles.push_back(std::make_unique<MoveParticleHandle<tool_t>>(*this));
  handles.push_back(std::make_unique<MoveAxisHandle<tool_t, MoveAxisHandleDirection::X>>(*this));
  handles.push_back(std::make_unique<MoveAxisHandle<tool_t, MoveAxisHandleDirection::Y>>(*this));
  position_variant.make_handles(this->handles, *this);
}

template<typename PositionVariant> bool SelectTool<PositionVariant>::has_transformation() const
{
  return !position_variant.is_empty();
}


SelectObjectsTool::SelectObjectsTool(Scene& scene) : SelectTool(scene)
{
  add_property<OptionsProperty>(TRANSFORMATION_MODE_KEY, 0)
    .set_options({ QObject::tr("Object").toStdString(), QObject::tr("Axis").toStdString() })
    .set_label(QObject::tr("Mode").toStdString())
    .set_category(QObject::tr("tool").toStdString());
}

std::string SelectObjectsTool::type() const { return TYPE; }
QIcon SelectObjectsTool::icon() const { return QIcon(); }

std::string SelectObjectsTool::name() const
{
  return QCoreApplication::translate("any-context", TYPE).toStdString();
}

Command* SelectObjectsTool::transform_objects(ObjectTransformation t, const bool tool_space)
{
  if (tool_space) {
    t = t.transformed(this->transformation().inverted());
  }

  const Matrix mat = viewport_transformation.to_mat().inverted() * t.to_mat();

  using TransformationMode = ObjectsTransformationCommand::TransformationMode;
  const auto tmode = property(TRANSFORMATION_MODE_KEY).value<TransformationMode>();
  auto command = std::make_unique<ObjectsTransformationCommand>( scene.item_selection<Object>(),
                                                                 mat, tmode );
  auto& command_ref = *command;
  scene.submit(std::move(command));
  return &command_ref;
}



SelectPointsTool::SelectPointsTool(Scene& scene)
  : SelectTool<PointPositions>(scene)
{
  add_property<OptionsProperty>(TANGENT_MODE_PROPERTY_KEY, 0)
    .set_options({ QObject::tr("Mirror").toStdString(), QObject::tr("Individual").toStdString() })
    .set_label(QObject::tr("tangent").toStdString())
    .set_category(QObject::tr("tool").toStdString());
}

std::string SelectPointsTool::type() const { return TYPE; }
QIcon SelectPointsTool::icon() const { return QIcon(); }

PointSelectHandle::TangentMode SelectPointsTool::tangent_mode() const
{
  const auto i = property(TANGENT_MODE_PROPERTY_KEY).value<size_t>();
  return static_cast<PointSelectHandle::TangentMode>(i);
}

std::unique_ptr<QMenu> SelectPointsTool::make_context_menu(QWidget* parent)
{
  Q_UNUSED(parent)
  auto& app = Application::instance();
  auto menus = app.key_bindings.make_menus(app, MainWindow::path_menu_entries());
  if (menus.size() > 1) {
    LWARNING << "cannot combine entries from multiple menus";
    // TODO replace top-level-menu with custom key (i.e. 'path' in this case).
    // then, all entries will be in the same menu.
  }
  if (menus.size() == 0) { return nullptr; }
  return std::move(menus.front());
}

void SelectPointsTool::on_selection_changed()
{
  on_scene_changed();
}

std::string SelectPointsTool::name() const
{
  return QCoreApplication::translate("any-context", TYPE).toStdString();
}

Command* SelectPointsTool::transform_objects(ObjectTransformation t, const bool tool_space)
{
  if (tool_space) { t = t.transformed(this->transformation().inverted()); }
  if (const auto paths = position_variant.paths(); paths.size() > 0) {
    auto command = std::make_unique<PointsTransformationCommand>(paths, t);
    auto& command_ref = *command;
    scene.submit(std::move(command));
    return &command_ref;
  } else {
    return nullptr;
  }
}

template class SelectTool<ObjectPositions>;
template class SelectTool<PointPositions>;

}  // namespace omm
