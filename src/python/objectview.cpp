#include "python/objectview.h"
#include "python/tagview.h"

omm::ObjectView::ObjectView(omm::Object& object)
  : View(object)
{
}

py::object omm::ObjectView::children()
{
  throw_if_no_target();

  std::vector<ObjectView> children;
  for (const auto& child : m_target->children()) {
    children.push_back(ObjectView(*child));
  }
  return py::cast(children);
}

py::object omm::ObjectView::parent()
{
  throw_if_no_target();

  if (m_target == nullptr) {
    throw std::runtime_error("Accessed Invalid Object");
  }

  if (m_target->is_root()) {
    return py::none();
  } else {
    return py::cast(ObjectView(m_target->parent()));
  }
}

py::object omm::ObjectView::tags()
{
  throw_if_no_target();

  std::vector<TagView> tags;
  for (const auto& tag : m_target->tags()) {
    tags.push_back(TagView(*tag));
  }
  return py::cast(tags);
}

void omm::ObjectView::remove()
{
  throw_if_no_target();

  if (m_target->is_root()) {
    throw std::runtime_error("Cannot remove root object");
  } else {
    m_target->parent().repudiate(*m_target);
  }
  m_target = nullptr;
}
