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

namespace omm
{

// TODO 1) implement a kd-tree or similar to get the closest object fast
//      2) I guess it's more intuitive when items can be selected by clicking anywhere inside
//          their area.
//      3) improve mouse pointer icon


template<typename PositionVariant> SelectTool<PositionVariant>::SelectTool(Scene& scene)
  : Tool(scene)
  , position_variant(scene)
{
  this->template add_property<OptionsProperty>(ALIGNMENT_PROPERTY_KEY)
    .set_options({ QObject::tr("global").toStdString(), QObject::tr("local").toStdString() })
    .set_label(QObject::tr("Alignment").toStdString())
    .set_category(QObject::tr("tool").toStdString());
}

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
::mouse_press(const arma::vec2& pos, const QMouseEvent& event, bool force)
{
  Q_UNUSED(force);
  if (Tool::mouse_press(pos, event, false)) {
    return true;
  } else if (Tool::mouse_press(pos, event, true)) {
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

void SelectObjectsTool::transform_objects(ObjectTransformation t, const bool tool_space)
{
  if (tool_space) { t = t.transformed(this->transformation().inverted()); }
  using TransformationMode = ObjectsTransformationCommand::TransformationMode;
  const auto tmode = property(TRANSFORMATION_MODE_KEY).value<TransformationMode>();
  scene.submit<ObjectsTransformationCommand>(scene.item_selection<Object>(), t, tmode);
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
  Q_UNUSED(parent);
  auto& app = Application::instance();
  auto menus = app.key_bindings.make_menus(app, MainWindow::path_menu_entries());
  if (menus.size() > 1) {
    LOG(WARNING) << "cannot combine entries from multiple menus";
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

void SelectPointsTool::transform_objects(ObjectTransformation t, const bool tool_space)
{
  if (tool_space) { t = t.transformed(this->transformation().inverted()); }
  if (const auto paths = position_variant.paths(); paths.size() > 0) {
    scene.submit<PointsTransformationCommand>(paths, t);
  }
}

template class SelectTool<ObjectPositions>;
template class SelectTool<PointPositions>;

}  // namespace omm
