#include "python/tagview.h"
#include "python/objectview.h"
#include "tags/tag.h"

omm::TagView::TagView(omm::Tag& target)
  : View(target)
{
}
