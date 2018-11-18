#include "mainwindow/viewport/handle.h"
#include <glog/logging.h>

#include "objects/object.h"
#include "mainwindow/viewport/subhandle.h"

namespace
{

auto make_sub_handles()
{
  std::vector<std::unique_ptr<omm::SubHandle>> sub_handles;
  sub_handles.push_back(std::make_unique<omm::AxisHandle>(omm::AxisHandle::Axis::X));
  sub_handles.push_back(std::make_unique<omm::AxisHandle>(omm::AxisHandle::Axis::Y));
  return sub_handles;
}

}  // namespace

namespace omm
{

Handle::Handle(const std::set<Object*>& selection)
  : m_sub_handles(make_sub_handles())
  , m_objects(selection)
{
}

bool Handle::mouse_move(const arma::vec2& delta)
{
  return false;
}

bool Handle::mouse_press(const arma::vec2& pos)
{
  return false;
}

void Handle::mouse_release()
{
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
  const auto add = [](const arma::vec2& accu, const Object* object) -> arma::vec2 {
    return accu + object->global_transformation().translation();
  };
  const auto sum_pos = std::accumulate(objects().begin(), objects().end(), arma::vec2(), add);
  return ObjectTransformation().translated(sum_pos / objects().size());
}

}  // namespace omm
