#include "python/tagview.h"
#include "python/objectview.h"
#include "tags/tag.h"

omm::TagView::TagView(omm::Tag& target)
  : View(target)
{
}

py::object omm::TagView::owner()
{
  throw_if_no_target();
  return py::cast(ObjectView(m_target->owner()));
}

void omm::TagView::remove()
{
  throw_if_no_target();
  m_target->owner().remove_tag(*m_target);
}