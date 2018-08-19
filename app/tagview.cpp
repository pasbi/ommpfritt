#include "tagview.h"
#include "objectview.h"
#include "tag.h"

TagView::TagView(Tag& target)
  : View(target)
{ 
}

py::object TagView::owner()
{
  throw_if_no_target();
  return py::cast(ObjectView(m_target->owner()));
}

void TagView::remove()
{
  throw_if_no_target();
  m_target->owner().remove_tag(*m_target);  
}