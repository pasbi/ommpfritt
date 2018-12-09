#include "mainwindow/viewport/handle.h"
#include <glog/logging.h>

#include "objects/object.h"
#include "mainwindow/viewport/axishandle.h"
#include "mainwindow/viewport/circlehandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"

namespace
{

auto make_sub_handles(omm::Handle& handle)
{
  std::vector<std::unique_ptr<omm::SubHandle>> sub_handles;
  sub_handles.push_back(std::make_unique<omm::XTranslateHandle>(handle));
  sub_handles.push_back(std::make_unique<omm::YTranslateHandle>(handle));
  sub_handles.push_back(std::make_unique<omm::XScaleHandle>(handle));
  sub_handles.push_back(std::make_unique<omm::YScaleHandle>(handle));
  sub_handles.push_back(std::make_unique<omm::ScaleHandle>(handle));
  sub_handles.push_back(std::make_unique<omm::CircleHandle>(handle));
  return sub_handles;
}

auto filter_selection(std::set<omm::Object*> selection)
{
  const auto has_descendant = [selection](omm::Object* subject) {
    const auto is_descendant = [subject](omm::Object* descendat_candidate) {
      return descendat_candidate != subject && descendat_candidate->is_descendant_of(*subject);
    };
    return std::any_of(selection.begin(), selection.end(), is_descendant);
  };

  decltype(selection) marked_for_deletion;
  std::copy_if( selection.begin(), selection.end(),
                std::inserter(marked_for_deletion, marked_for_deletion.end()), has_descendant );
  for (omm::Object* object : marked_for_deletion) {
    selection.erase(object);
  }
  return selection;
}

}  // namespace

namespace omm
{

Handle::Handle(Scene& scene, const std::set<Object*>& selection)
  : m_scene(scene)
  , m_sub_handles(make_sub_handles(*this))
  , m_objects(filter_selection(selection))
{
}

bool Handle::mouse_move(const arma::vec2& delta, const arma::vec2& pos)
{
  bool hit_something = false;
  if (objects().size() > 0) {
    const auto handle_local_pos = map_to_handle_local(pos);
    for (const auto& sub_handle : m_sub_handles) {
      sub_handle->mouse_move(delta, handle_local_pos, !hit_something);
      switch (sub_handle->status()) {
      case SubHandle::Status::Active:
      case SubHandle::Status::Hovered:
        hit_something = true;
        break;
      case SubHandle::Status::Inactive:
        break;
      }
    }
  }
  return hit_something;
}

bool Handle::mouse_press(const arma::vec2& pos)
{
  if (objects().size() > 0) {
    const auto handle_local_pos = map_to_handle_local(pos);
    for (const auto& sub_handle : m_sub_handles) {
      sub_handle->mouse_press(handle_local_pos);
      if (sub_handle->status() == SubHandle::Status::Active) {
        return true;
      }
    }
  }
  return false;
}

void Handle::mouse_release()
{
  if (objects().size() > 0) {
    for (const auto& sub_handle : m_sub_handles) {
      sub_handle->mouse_release();
    }
  }
}

void Handle::draw(AbstractRenderer& renderer) const
{
  if (objects().size() > 0) {
    Style style;
    const auto transformation = this->transformation();
    renderer.push_transformation(transformation);
    for (const auto& sub_handle : m_sub_handles) {
      sub_handle->draw(renderer);
    }
    renderer.pop_transformation();
  }
}

const std::set<Object*>& Handle::objects() const
{
  return m_objects;
}


ObjectTransformation GlobalOrientedHandle::transformation() const
{
  assert(objects().size() > 0);
  const auto add = [](const arma::vec2& accu, const Object* object) -> arma::vec2 {
    return accu + object->global_transformation().translation();
  };
  const auto sum_pos = std::accumulate( objects().begin(), objects().end(),
                                        arma::vec2({0.0, 0.0}), add );
  return ObjectTransformation().translated(sum_pos / objects().size());
}

arma::vec2 Handle::map_to_handle_local(const arma::vec2& pos) const
{
  return transformation().inverted().apply_to_position(pos);
}

void Handle::transform_objects(const ObjectTransformation& transformation) const
{
  const ObjectTransformation::Mat h = this->transformation().to_mat();
  const ObjectTransformation::Mat h_inv = h.i();
  const ObjectTransformation::Mat t = transformation.to_mat();

  m_scene.submit<ObjectsTransformationCommand>(objects(), ObjectTransformation(h * t * h_inv));
}

}  // namespace omm
