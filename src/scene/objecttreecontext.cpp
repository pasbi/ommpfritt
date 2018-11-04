#include "scene/objecttreecontext.h"
#include "objects/object.h"

namespace omm
{

ObjectTreeContext::ObjectTreeContext(Object& subject)
  : ObjectTreeContext(subject, subject.parent(), subject.predecessor())
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
  const bool parent_does_not_change = &parent.get() == &subject.get().parent();
  const bool predecessor_did_not_change = sibling_before == subject.get().predecessor();

  // the `parent_does_not_change test` is only required if `sibling_before == nullptr`.
  return parent_does_not_change && predecessor_did_not_change;
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
