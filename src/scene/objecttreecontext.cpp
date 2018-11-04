#include "scene/objecttreecontext.h"
#include "objects/object.h"

namespace omm
{

ObjectTreeContext::ObjectTreeContext(Object& subject)
  : subject(subject)
  , parent(subject.parent())
  , sibling_before(subject.predecessor())
{
}

ObjectTreeContext::ObjectTreeContext(Object& subject, Object& parent, const Object* sibling_before)
  : subject(subject)
  , parent(parent)
  , sibling_before(sibling_before)
{
}

size_t ObjectTreeContext::get_insert_position() const
{
  return parent.get().get_insert_position(sibling_before);
}

bool ObjectTreeContext::is_root() const
{
  return subject.get().is_root();
}

bool ObjectTreeContext::move_into_itself() const
{
  // subject cannot become its own parent (illogical)
  return &parent.get() == &subject.get();
}

bool ObjectTreeContext::move_before_itself() const
{
  return sibling_before == subject.get().predecessor();
}

bool ObjectTreeContext::move_after_itself() const
{
  return sibling_before == &subject.get();
}

bool ObjectTreeContext::is_valid() const
{
  return !is_root() && !move_into_itself() && !move_after_itself();
}

bool ObjectTreeContext::is_strictly_valid() const
{
  return is_valid() && !move_before_itself();
}

}  // namespace omm
