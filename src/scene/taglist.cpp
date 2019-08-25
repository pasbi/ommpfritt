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
  Q_EMIT m_object.appearance_changed(&m_object);
}

void TagList::remove(ListOwningContext<Tag> &t)
{
  List<Tag>::remove(t);
  Q_EMIT m_object.appearance_changed(&m_object);
}

std::unique_ptr<Tag> TagList::remove(Tag &tag)
{
  auto otag = List<Tag>::remove(tag);
  Q_EMIT m_object.appearance_changed(&m_object);
  return otag;
}

void TagList::move(ListMoveContext<Tag> &context)
{
  List<Tag>::move(context);
  Q_EMIT m_object.appearance_changed(&m_object);
}

}  // namespace omm
