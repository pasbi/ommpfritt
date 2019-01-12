#include "tools/itemtools/selecttool.h"
#include <memory>
#include <algorithm>
#include "tools/handles/axishandle.h"
#include "tools/handles/particlehandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"
#include "objects/path.h"
#include "properties/optionsproperty.h"

namespace omm
{

// TODO 1) implement a kd-tree or similar to get the closest object fast
//      2) I guess it's more intuitive when items can be selected by clicking anywhere inside
//          its area.
//      3) improve mouse pointer icon
//      4) select during mouse move (like drawing)

template<typename PositionVariant> bool SelectTool<PositionVariant>
::mouse_press( const arma::vec2& pos, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
  if (!Tool::mouse_press(pos, buttons, modifiers)) {
    m_position_variant.clear_selection();
  }
  return true;
}

template<typename PositionVariant>
void SelectTool<PositionVariant>::on_scene_changed()
{
  this->handles.clear();
  m_position_variant.make_handles(this->handles, *this);
}

template<typename PositionVariant>
void SelectTool<PositionVariant>::transform_objects(const ObjectTransformation& transformation)
{
  m_position_variant.transform(transformation);
}


std::string SelectObjectsTool::type() const
{
  return TYPE;
}

QIcon SelectObjectsTool::icon() const
{
  return QIcon();
}

SelectPointsTool::SelectPointsTool(Scene& scene)
  : SelectTool<PointPositions>(scene)
{
  add_property<OptionsProperty>(TANGENT_MODE_PROPERTY_KEY, 0)
    .set_options({ "Mirror", "Individual" })
    .set_label(QObject::tr("tangent").toStdString())
    .set_category(QObject::tr("tool").toStdString());
}

std::string SelectPointsTool::type() const
{
  return TYPE;
}

QIcon SelectPointsTool::icon() const
{
  return QIcon();
}

void SelectPointsTool::on_selection_changed()
{
  on_scene_changed();
}

PointSelectHandle::TangentMode SelectPointsTool::tangent_mode() const
{
  const auto i = property(TANGENT_MODE_PROPERTY_KEY).value<size_t>();
  return static_cast<PointSelectHandle::TangentMode>(i);
}

template class SelectTool<ObjectPositions>;
template class SelectTool<PointPositions>;

}  // namespace omm
