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
  for (const std::unique_ptr<Object>& child : m_target->m_children) {
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
    return py::cast(ObjectView(*m_target->m_parent));
  }
}

py::object omm::ObjectView::tags()
{
  throw_if_no_target();

  std::vector<TagView> tags;
  for (const std::unique_ptr<Tag>& tag : m_target->m_tags) {
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

py::object omm::ObjectView::create_child()
{
  throw_if_no_target();

  Object& object = m_target->create_child<Object>();
  return py::cast(ObjectView(object));
}

py::object omm::ObjectView::create_tag()
{
  throw_if_no_target();

  Tag& tag = m_target->create_tag<Tag>();
  return py::cast(TagView(tag));
}