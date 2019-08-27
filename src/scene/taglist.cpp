#include "scene/taglist.h"
#include "scene/scene.h"
#include "objects/object.h"
#include "tags/tag.h"

namespace omm
{

TagList::TagList(Object &object) : QObject(), m_object(object)
{
}

TagList::TagList(const TagList &other, Object& object)
  : QObject()
  , List<Tag>(other)
  , m_object(object)
{
}

void TagList::insert(ListOwningContext<Tag> &context)
{
  List<Tag>::insert(context);
  Q_EMIT tag_inserted(context.subject);
}

void TagList::remove(ListOwningContext<Tag> &t)
{
  List<Tag>::remove(t);
  Q_EMIT tag_removed(t.subject.get());
}

std::unique_ptr<Tag> TagList::remove(Tag &tag)
{
  auto otag = List<Tag>::remove(tag);
  Q_EMIT tag_removed(tag);
  return otag;
}

void TagList::move(ListMoveContext<Tag> &context)
{
  Q_UNUSED(context);
  Q_UNREACHABLE();
}

}  // namespace omm
