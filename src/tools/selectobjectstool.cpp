#include "tools/selectobjectstool.h"
#include "commands/objectstransformationcommand.h"
#include "tools/handles/moveaxishandle.h"
#include "tools/handles/rotatehandle.h"
#include "tools/handles/scalebandhandle.h"
#include "tools/handles/particlehandle.h"
#include "scene/scene.h"
#include "properties/optionsproperty.h"

namespace omm
{

SelectObjectsTool::SelectObjectsTool(Scene& scene) : AbstractSelectTool(scene)
{
  add_property<OptionsProperty>(TRANSFORMATION_MODE_KEY, 0)
    .set_options({ QObject::tr("Object").toStdString(), QObject::tr("Axis").toStdString() })
    .set_label(QObject::tr("Mode").toStdString())
    .set_category(QObject::tr("tool").toStdString());
}

std::string SelectObjectsTool::type() const { return TYPE; }

Command* SelectObjectsTool::transform_objects(ObjectTransformation t, const bool tool_space)
{
  if (tool_space) {
    t = t.transformed(this->transformation().inverted());
  }

  const Matrix mat = viewport_transformation.to_mat().inverted() * t.to_mat();

  using TransformationMode = ObjectsTransformationCommand::TransformationMode;
  const auto tmode = property(TRANSFORMATION_MODE_KEY)->value<TransformationMode>();
  auto command = std::make_unique<ObjectsTransformationCommand>( scene.item_selection<Object>(),
                                                                 mat, tmode );
  auto& command_ref = *command;
  scene.submit(std::move(command));
  return &command_ref;
}

void SelectObjectsTool::on_scene_changed()
{
  handles.clear();
  using tool_t = std::remove_pointer_t<decltype(this)>;
  handles.push_back(std::make_unique<ScaleBandHandle<tool_t>>(*this));
  handles.push_back(std::make_unique<RotateHandle<tool_t>>(*this));
  handles.push_back(std::make_unique<MoveParticleHandle<tool_t>>(*this));
  handles.push_back(std::make_unique<MoveAxisHandle<tool_t, MoveAxisHandleDirection::X>>(*this));
  handles.push_back(std::make_unique<MoveAxisHandle<tool_t, MoveAxisHandleDirection::Y>>(*this));


  // ignore object selection. Return a handle for each visible object.
  const auto objects = ::filter_if(scene.object_tree.items(), [](Object* object) {
    return object->is_visible();
  });

  handles.reserve(objects.size());
  auto inserter = std::back_inserter(handles);
  std::transform(objects.begin(), objects.end(), inserter, [this](Object* o) {
    return std::make_unique<ObjectSelectHandle>(*this, scene, *o);
  });
}


bool SelectObjectsTool::mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force)
{
  Q_UNUSED(force);
  if (AbstractSelectTool::mouse_press(pos, event, false)) {
    return true;
  } else if (AbstractSelectTool::mouse_press(pos, event, true)) {
    return true;
  } else {
    scene.set_selection({});
    return false;
  }
}

bool SelectObjectsTool::has_transformation() const
{
  return !scene.item_selection<Object>().empty();
}

Vec2f SelectObjectsTool::selection_center() const
{
  const auto objects = scene.item_selection<Object>();

  Vec2f sum(0.0, 0.0);
  for (const auto& o : objects) {
    sum += o->global_transformation().translation();
  }

  return sum / static_cast<double>(objects.size());
}

}  // namespace omm
