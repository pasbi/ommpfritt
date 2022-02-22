#include "tools/selectobjectstool.h"
#include "commands/objectstransformationcommand.h"
#include "properties/optionproperty.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "tools/handles/boundingboxhandle.h"
#include "tools/handles/moveaxishandle.h"
#include "tools/handles/objectselecthandle.h"
#include "tools/handles/particlehandle.h"
#include "tools/handles/rotatehandle.h"
#include "tools/handles/scaleaxishandle.h"
#include "tools/handles/scalebandhandle.h"
#include "tools/transformobjectshelper.h"
#include "removeif.h"

namespace omm
{
SelectObjectsTool::SelectObjectsTool(Scene& scene)
  : AbstractSelectTool(scene)
  , m_transform_objects_helper(std::make_unique<TransformObjectsHelper>())
{
  create_property<OptionProperty>(TRANSFORMATION_MODE_KEY, 0)
      .set_options({QObject::tr("Object"), QObject::tr("Axis")})
      .set_label(QObject::tr("Mode"))
      .set_category(QObject::tr("tool"))
      .set_animatable(false);
}

SelectObjectsTool::~SelectObjectsTool() = default;

QString SelectObjectsTool::type() const
{
  return TYPE;
}

void SelectObjectsTool::transform_objects(ObjectTransformation t)
{
  const Matrix viewport_mat = viewport_transformation.to_mat();
  const Matrix inv_viewport_mat = viewport_mat.inverted();
  const Matrix premul = inv_viewport_mat * t.to_mat() * viewport_mat;

  using TransformationMode = ObjectsTransformationCommand::TransformationMode;
  const auto mode = property(TRANSFORMATION_MODE_KEY)->value<TransformationMode>();
  scene()->submit(m_transform_objects_helper->make_command(premul, mode));
}

void SelectObjectsTool::reset()
{
  clear();
  using tool_t = std::remove_pointer_t<decltype(this)>;
  push_handle(std::make_unique<ScaleBandHandle<tool_t>>(*this));
  push_handle(std::make_unique<RotateHandle<tool_t>>(*this));
  push_handle(std::make_unique<MoveAxisHandle<tool_t, AxisHandleDirection::X>>(*this));
  push_handle(std::make_unique<MoveAxisHandle<tool_t, AxisHandleDirection::Y>>(*this));
  push_handle(std::make_unique<ScaleAxisHandle<tool_t, AxisHandleDirection::X>>(*this));
  push_handle(std::make_unique<ScaleAxisHandle<tool_t, AxisHandleDirection::Y>>(*this));
  push_handle(std::make_unique<BoundingBoxHandle<SelectObjectsTool>>(*this));

  // ignore object selection. Return a handle for each visible object.
  const auto objects = util::remove_if(scene()->object_tree().items(), [](Object* object) {
    return !object->is_visible(true);
  });

  for (const auto& object : objects) {
    push_handle(std::make_unique<ObjectSelectHandle>(*this, *scene(), *object));
  }
  push_handle(std::make_unique<MoveParticleHandle<tool_t>>(*this));
}

bool SelectObjectsTool::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  if (AbstractSelectTool::mouse_press(pos, event)) {
    m_transform_objects_helper->update(scene()->item_selection<Object>());
    return true;
  } else {
    scene()->set_selection({});
    return false;
  }
}

void SelectObjectsTool::mouse_release(const Vec2f& pos, const QMouseEvent& event)
{
  AbstractSelectTool::mouse_release(pos, event);
  const auto items = scene()->item_selection<Object>();
  scene()->emit_selection_changed_signal();
}

BoundingBox SelectObjectsTool::bounding_box() const
{
  if (scene()->item_selection<Object>().empty()) {
    return BoundingBox();
  } else {
    return BoundingBox::around_selected_objects(*scene());
  }
}

Vec2f SelectObjectsTool::selection_center() const
{
  const auto objects = scene()->item_selection<Object>();

  Vec2f sum(0.0, 0.0);
  for (const auto& o : objects) {
    sum += o->global_transformation(Space::Viewport).translation();
  }
  return sum / static_cast<double>(objects.size());
}

}  // namespace omm
