#include "tools/objecttransformationtool.h"
#include <memory>
#include "tools/handles/axishandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"

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

void ObjectTransformationTool::transform_objects(const ObjectTransformation& transformation)
{
  const ObjectTransformation::Mat h = this->transformation().to_mat();
  const ObjectTransformation::Mat h_inv = h.i();
  const ObjectTransformation::Mat t = transformation.to_mat();

  const auto global = ObjectTransformation(h * t * h_inv);
  scene.submit<ObjectsTransformationCommand>(selection(), global);
}

bool ObjectTransformationTool::mouse_move(const arma::vec2& delta, const arma::vec2& pos)
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

bool ObjectTransformationTool::mouse_press(const arma::vec2& pos)
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

void ObjectTransformationTool::mouse_release()
{
  if (selection().size() > 0) {
    for (auto* handle : handles()) {
      handle->mouse_release();
    }
  }
}


}  // namespace omm
