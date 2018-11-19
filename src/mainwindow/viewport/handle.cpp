#include "mainwindow/viewport/handle.h"
#include <glog/logging.h>

#include "objects/object.h"
#include "mainwindow/viewport/subhandle.h"

namespace
{

auto make_sub_handles(omm::Handle& handle)
{
  std::vector<std::unique_ptr<omm::SubHandle>> sub_handles;
  sub_handles.push_back(std::make_unique<omm::AxisHandle>(handle, omm::AxisHandle::Axis::X));
  sub_handles.push_back(std::make_unique<omm::AxisHandle>(handle, omm::AxisHandle::Axis::Y));
  return sub_handles;
}

}  // namespace

namespace omm
{

Handle::Handle(const std::set<Object*>& selection)
  : m_sub_handles(make_sub_handles(*this))
  , m_objects(selection)
{
}

bool Handle::mouse_move(const arma::vec2& delta, const arma::vec2& pos)
{
  if (objects().size() > 0) {
    const auto handle_local_pos = map_to_handle_local(pos);
    for (const auto& sub_handle : m_sub_handles) {
      if (sub_handle->mouse_move(delta, handle_local_pos)) {
        return true;
      }
    }
  }
  return false;
}

bool Handle::mouse_press(const arma::vec2& pos)
{
  if (objects().size() > 0) {
    const auto handle_local_pos = map_to_handle_local(pos);
    for (const auto& sub_handle : m_sub_handles) {
      if (sub_handle->mouse_press(handle_local_pos)) {
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
  const auto sum_pos = std::accumulate(objects().begin(), objects().end(), arma::vec2(), add);
  return ObjectTransformation().translated(sum_pos / objects().size());
}

arma::vec2 Handle::map_to_handle_local(const arma::vec2& pos) const
{
  return transformation().inverted().apply_to_position(pos);
}

void Handle::transform_objects(const ObjectTransformation& transformation) const
{
  for (auto* object : objects()) {
    object->set_global_transformation(transformation.apply(object->global_transformation()));
  }
}

}  // namespace omm
