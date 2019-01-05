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
{
  this->handles = std::move(handles);
  add_property<OptionsProperty>(ALIGNMENT_PROPERTY_KEY)
    .set_options({ "global", "local" })
    .set_label(QObject::tr("Alignment").toStdString())
    .set_category(QObject::tr("Tool").toStdString());
}

ObjectTransformation ObjectsTool::transformation() const
{
  const auto selection = scene.object_selection();
  ObjectTransformation transformation;
  transformation.translate(get_global_position_mean(selection));
  if (property(ALIGNMENT_PROPERTY_KEY).value<size_t>() == 1 && selection.size() == 1) {
      transformation.rotate((*selection.begin())->global_transformation().rotation());
  }
  return transformation;
}

void ObjectsTool::draw(AbstractRenderer& renderer) const
{
  if (scene.object_selection().size() == 0) {
    return;
  }

  renderer.push_transformation(this->transformation());
  Tool::draw(renderer);
  renderer.pop_transformation();
}

void ObjectsTool::transform_objects(const ObjectTransformation& transformation)
{
  const ObjectTransformation::Mat h = this->transformation().to_mat();
  const ObjectTransformation::Mat h_inv = h.i();
  const ObjectTransformation::Mat t = transformation.to_mat();

  const auto global = ObjectTransformation(h * t * h_inv);
  scene.submit<ObjectsTransformationCommand>(scene.object_selection(), global);
}

bool ObjectsTool::mouse_move(const arma::vec2& delta, const arma::vec2& pos)
{
  if (scene.object_selection().size() > 0) {
    const auto t_inv = transformation().inverted();
    const auto local_pos = t_inv.apply_to_position(pos);
    const auto local_delta = t_inv.apply_to_direction(delta);
    return Tool::mouse_move(local_delta, local_pos);
  } else {
    return false;
  }
}

bool ObjectsTool::mouse_press(const arma::vec2& pos)
{
  if (scene.object_selection().size() > 0) {
    const auto t_inv = transformation().inverted();
    const auto local_pos = t_inv.apply_to_position(pos);
    return Tool::mouse_press(local_pos);
  }
  return false;
}

void ObjectsTool::mouse_release()
{
  if (scene.object_selection().size() > 0) {
    for (auto&& handle : handles) {
      handle->mouse_release();
    }
  }
}

}  // namespace omm
