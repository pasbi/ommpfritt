#include "tools/selectobjectstool.h"
#include "commands/objectstransformationcommand.h"
#include "tools/handles/moveaxishandle.h"
#include "tools/handles/rotatehandle.h"
#include "tools/handles/scalebandhandle.h"
#include "tools/handles/particlehandle.h"
#include "scene/scene.h"
#include "properties/optionsproperty.h"
#include "tools/handles/boundingboxhandle.h"

namespace omm
{

SelectObjectsTool::SelectObjectsTool(Scene& scene)
  : AbstractSelectTool(scene)
{
  create_property<OptionsProperty>(TRANSFORMATION_MODE_KEY, 0)
    .set_options({ QObject::tr("Object"), QObject::tr("Axis") })
    .set_label(QObject::tr("Mode"))
    .set_category(QObject::tr("tool"))
    .set_animatable(false);
}

QString SelectObjectsTool::type() const { return TYPE; }

void SelectObjectsTool::transform_objects(ObjectTransformation t)
{
  const Matrix viewport_mat = viewport_transformation.to_mat();
  const Matrix inv_viewport_mat = viewport_mat.inverted();
  const Matrix premul = inv_viewport_mat * t.to_mat() * viewport_mat;

  using TransformationMode = ObjectsTransformationCommand::TransformationMode;
  const auto mode = property(TRANSFORMATION_MODE_KEY)->value<TransformationMode>();
  scene()->submit(m_transform_objects_helper.make_command(premul, mode));
}

void SelectObjectsTool::reset()
{
  handles.clear();
  using tool_t = std::remove_pointer_t<decltype(this)>;
  handles.push_back(std::make_unique<ScaleBandHandle<tool_t>>(*this));
  handles.push_back(std::make_unique<RotateHandle<tool_t>>(*this));
  handles.push_back(std::make_unique<MoveParticleHandle<tool_t>>(*this));
  handles.push_back(std::make_unique<MoveAxisHandle<tool_t, MoveAxisHandleDirection::X>>(*this));
  handles.push_back(std::make_unique<MoveAxisHandle<tool_t, MoveAxisHandleDirection::Y>>(*this));
  handles.push_back(std::make_unique<BoundingBoxHandle<SelectObjectsTool>>(*this));

  // ignore object selection. Return a handle for each visible object.
  const auto objects = ::filter_if(scene()->object_tree().items(), [](Object* object) {
    return object->is_visible();
  });

  handles.reserve(objects.size());
  auto inserter = std::back_inserter(handles);
  std::transform(objects.begin(), objects.end(), inserter, [this](Object* o) {
    return std::make_unique<ObjectSelectHandle>(*this, *scene(), *o);
  });
}

bool SelectObjectsTool::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  if (AbstractSelectTool::mouse_press(pos, event)) {
    m_transform_objects_helper.update(scene()->item_selection<Object>());
    return true;
  } else {
    scene()->set_selection({});
    return false;
  }
}

BoundingBox SelectObjectsTool::bounding_box() const
{
  if (scene()->item_selection<Object>().size() == 0) {
    return BoundingBox();
  } else {
    return BoundingBox::around_selected_objects(*scene());
  }
}

bool SelectObjectsTool::has_transformation() const
{
  return !scene()->item_selection<Object>().empty();
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

TransformObjectsHelper::TransformObjectsHelper()
{
  update();
}

std::unique_ptr<ObjectsTransformationCommand>
TransformObjectsHelper::make_command(const Matrix &t, TransformationMode mode) const
{
  ObjectsTransformationCommand::Map map;
  for (auto&& [object, transformation] : m_initial_transformations) {
    map.insert(std::pair(object, ObjectTransformation(t * transformation.to_mat())));
  }
  return std::make_unique<ObjectsTransformationCommand>( map, mode );
}

void TransformObjectsHelper::update(const std::set<Object *> &objects)
{
  m_objects = objects;
  update();
}

void TransformObjectsHelper::update()
{
  m_initial_transformations.clear();
  for (auto&& o : m_objects) {
    m_initial_transformations.insert(std::pair(o, o->global_transformation(Space::Scene)));
  }
}

}  // namespace omm
