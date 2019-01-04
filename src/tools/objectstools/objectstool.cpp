#include "tools/objectstools/objectstool.h"

#include <memory>
#include "renderers/abstractrenderer.h"
#include "objects/object.h"
#include "properties/optionsproperty.h"
#include "commands/objectstransformationcommand.h"
#include "scene/scene.h"
#include "common.h"

namespace
{

arma::vec2 get_global_position_mean(const std::set<omm::Object*>& objects)
{
  assert(objects.size() > 0);
  const auto add = [](const arma::vec2& accu, const omm::Object* object) -> arma::vec2 {
    return accu + object->global_transformation().translation();
  };
  const auto null = arma::vec2 {0.0, 0.0};
  return std::accumulate(objects.begin(), objects.end(), null, add) / objects.size();
}

}  // namespace

namespace omm
{

ObjectsTool::ObjectsTool(Scene& scene, std::vector<std::unique_ptr<Handle>> handles)
  : Tool(scene)
  , m_handles(std::move(handles))
{
  add_property<OptionsProperty>(ALIGNMENT_PROPERTY_KEY)
    .set_options({ "global", "local" })
    .set_label(QObject::tr("Alignment").toStdString())
    .set_category(QObject::tr("Tool").toStdString());
}

ObjectTransformation ObjectsTool::transformation() const
{
  ObjectTransformation transformation;
  transformation.translate(get_global_position_mean(selection()));
  if (property(ALIGNMENT_PROPERTY_KEY).value<size_t>() == 1 && selection().size() == 1) {
      transformation.rotate((*selection().begin())->global_transformation().rotation());
  }
  return transformation;
}

void ObjectsTool::draw(AbstractRenderer& renderer) const
{
  if (selection().size() == 0) {
    return;
  }

  renderer.push_transformation(this->transformation());

  for (const auto* handle : handles()) {
    handle->draw(renderer);
  }
  renderer.pop_transformation();
}

arma::vec2 ObjectsTool::map_to_tool_local(const arma::vec2& pos) const
{
  return transformation().inverted().apply_to_position(pos);
}

void ObjectsTool::transform_objects(const ObjectTransformation& transformation)
{
  const ObjectTransformation::Mat h = this->transformation().to_mat();
  const ObjectTransformation::Mat h_inv = h.i();
  const ObjectTransformation::Mat t = transformation.to_mat();

  const auto global = ObjectTransformation(h * t * h_inv);
  scene.submit<ObjectsTransformationCommand>(selection(), global);
}

bool ObjectsTool::mouse_move(const arma::vec2& delta, const arma::vec2& pos)
{
  bool hit_something = false;
  if (selection().size() > 0) {
    const auto handle_local_pos = map_to_tool_local(pos);
    for (auto* handle : handles()) {
      const auto transformed_delta = transformation().inverted().apply_to_direction(delta);
      handle->mouse_move(transformed_delta, handle_local_pos, !hit_something);
      switch (handle->status()) {
      case Handle::Status::Active:
      case Handle::Status::Hovered:
        hit_something = true;
        break;
      case Handle::Status::Inactive:
        break;
      }
    }
  }
  return hit_something;
}

bool ObjectsTool::mouse_press(const arma::vec2& pos)
{
  if (selection().size() > 0) {
    const auto handle_local_pos = map_to_tool_local(pos);
    for (auto* handle : handles()) {
      handle->mouse_press(handle_local_pos);
      if (handle->status() == Handle::Status::Active) {
        return true;
      }
    }
  }
  return false;
}

void ObjectsTool::mouse_release()
{
  if (selection().size() > 0) {
    for (auto* handle : handles()) {
      handle->mouse_release();
    }
  }
}

std::vector<Handle*> ObjectsTool::handles() const
{
  return ::transform<Handle*>(m_handles, std::mem_fn(&std::unique_ptr<Handle>::get));
}

}  // namespace omm
