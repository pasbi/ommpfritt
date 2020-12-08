#include "scene/taglist.h"
#include "objects/object.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "tags/tag.h"

namespace omm
{
TagList::TagList(Object& object) : m_object(object)
{
}

TagList::TagList(const TagList& other, Object& object) : List<Tag>(other), m_object(object)
{
}

void TagList::insert(ListOwningContext<Tag>& context)
{
  List<Tag>::insert(context);
  Q_EMIT scene().mail_box().tag_inserted(m_object, context.get_subject());
}

void TagList::remove(ListOwningContext<Tag>& t)
{
  List<Tag>::remove(t);
  Q_EMIT scene().mail_box().tag_removed(m_object, t.get_subject());
}

std::unique_ptr<Tag> TagList::remove(Tag& tag)
{
  Object& owner = *tag.owner;
  auto otag = List<Tag>::remove(tag);
  Q_EMIT scene().mail_box().tag_removed(owner, tag);
  return otag;
}

void TagList::move(ListMoveContext<Tag>& context)
{
  Q_UNUSED(context);
  Q_UNREACHABLE();
}

Scene& TagList::scene()
{
  return *m_object.scene();
}

}  // namespace omm
